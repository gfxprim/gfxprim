#!/usr/bin/env python3
#
# SPDX-License-Identifier: GPL-2.0-or-later

import xml.etree.ElementTree as ET
import sys, os, glob

def get_text(node):
    """Extracts text from leaf xml element."""
    if node is None: return ""
    return "".join(node.itertext()).strip().replace('\n', ' ')

def get_desc(node):
    """Extract description, formats C code, skips a list of parameters."""
    if node is None:
        return ""

    parts = []
    if node.text:
        parts.append(node.text.replace('\n', ' ').strip())

    for child in node:
        if child.tag in ['parameterlist', 'simplesect']:
            continue

        if child.tag in ['computeroutput', 'programlisting', 'verbatim']:
            code_parts = []

            def extract_code(node):
                if node.text:
                    code_parts.append(node.text)
                for sub in node:
                    if sub.tag == 'sp':
                        code_parts.append(' ')
                    extract_code(sub)
                    if sub.tail:
                        code_parts.append(sub.tail)

            extract_code(child)
            code_text = "".join(code_parts)

            parts.append("\n.sp\n.RS 4\n.nf\n")
            parts.append(code_text)
            parts.append("\n.fi\n.RE\n.sp\n")
        else:
            text_content = "".join(child.itertext()).replace('\n', ' ')
            parts.append(text_content)

        if child.tail:
            parts.append(child.tail.replace('\n', ' ').strip())

    return " ".join(parts).strip()

def clear_type(p_type):
    """Removes modifiers and keywords from a type e.g. 'enum gp_backend *' -> 'gp_backend'."""
    for word in ['const', 'enum', 'struct', 'volatile']:
        p_type = p_type.replace(word, '')

    return p_type.replace('*', '').replace('&', '').strip()

def safe_name(name):
    return name.replace(':', '_').replace(' ', '_')

def xml_to_man(xml_path, out_dir, all_enums, all_structs):
    """Iterates over all functions in a xml file and generates man page for each of them."""
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
    except Exception as e:
        return

    see_also_funcs = [m.findtext('name') for m in tree.findall(".//memberdef[@kind='function']")]
    see_also_macros = [m.findtext('name') for m in root.findall(".//memberdef[@kind='define']")
                       if m.find('param') is not None]

    # Generate docs for functions and macros with parameters
    for func in root.findall(".//memberdef[@kind='function']") + \
                root.findall(".//memberdef[@kind='define']"):
        name = func.findtext('name')
        header = os.path.basename(func.find('location').get('file'))

        if not name or not (name.startswith('gp_') or name.startswith('GP_')) or name.endswith('_'):
            continue

        print(f" GEN {name}.3")

        # SYNOPSIS
        content = [
            f'.TH "{name.upper()}" 3',
            f'.SH NAME',
            f'{name} \\- {get_text(func.find("briefdescription/para"))}',
            f'.SH SYNOPSIS',
            f'.nf',
            f'.B #include <{header}>',
            f'.sp',
        ]

        params_type = {}
        see_also = []

        is_macro = func.get('kind') == 'define'
        if is_macro:
            macro_params = []
            for p in func.findall('param'):
                p_n = p.findtext('defname')
                if p_n:
                    macro_params.append(p_n)

            if macro_params:
                formatted_p = ", ".join([f"\\fI{p}\\fR" for p in macro_params])
                content.append(f"\\fB#define {name}(\\fR{formatted_p}\\fB)\\fR")
            else:
                continue;
        else:
            type_node = func.find('type')
            ret_type = get_text(type_node) or "void"

            params_info = []

            for p in func.findall('param'):
                p_type = get_text(p.find('type'))
                p_name = p.findtext('declname') or ""
                par = f"\\fB{p_type}\\fR"
                if p_name:
                    if not p_type.endswith('*'):
                        par+=' '
                    par += f"\\fI{p_name}\\fR"
                    params_type[p_name] = p_type
                params_info.append(par)
            if ret_type.endswith('*'):
                sp = ''
            else:
                sp = ' '
            content.append(f"\\fB{ret_type}{sp}{name}(\\fR{', '.join(params_info)}\\fB);\\fR")
        content.append('.fi');

        # DESCRIPTION
        desc_nodes = para_nodes = func.findall('detaileddescription/para')
        if desc_nodes:
            desc_parts = [get_desc(p) for p in para_nodes]
            desc = "\n.sp\n".join(desc_parts)
            if desc:
                content.append('.SH DESCRIPTION\n' + desc)

        # ARGUMENTS
        param_list = func.find(".//parameterlist[@kind='param']")
        if param_list is not None:
            content.append('.SH ARGUMENTS')
            for item in param_list.findall('parameteritem'):
                p_name = get_text(item.find('.//parametername'))
                p_desc = get_text(item.find('.//parameterdescription/para'))
                p_type = ''

                if p_name in params_type:
                    p_type = params_type[p_name]
                else:
                    if not is_macro:
                        print(f"ERROR: param {p_name} not in the function {name} argument list!")

                if p_type.endswith('*'):
                    sp = ''
                else:
                    sp = ' '

                content.append(f'.TP\n\\fB{p_type}{sp}\\fI{p_name}\\fR\n{p_desc}')
                p_ctype = clear_type(p_type);
                # If parameter is structure and we have doc for it, link it
                if p_ctype in all_structs:
                    see_also.append(p_ctype)
                # If parameter is enum, include full description
                if p_ctype in all_enums:
                    content.append('.sp')
                    content.append('.RS 4')
                    content.append(all_enums[p_ctype]['description'] + "\n")
                    for v_name, v_desc in all_enums[p_ctype]['items']:
                        content.append(f'.IP "\\fB{v_name}\\fR" 15\n{v_desc or "No description."}')
                    content.append('.RE')

        # RETURN VALUE
        ret_node = func.find(".//simplesect[@kind='return']")
        if ret_node is not None:
            content.append(f'.SH RETURN VALUE\n{get_text(ret_node.find("para"))}')

        # SEE ALSO
        see_also += [f for f in see_also_funcs if f != name]
        see_also += [f for f in see_also_macros if f != name]

        if see_also:
            content.append('.SH SEE ALSO')
            links = ", ".join([f"\\fB{f}\\fR(3)" for f in see_also])
            content.append(links)

        with open(os.path.join(out_dir, f"{safe_name(name)}.3"), "w", encoding="utf-8") as f:
            f.write("\n".join(content))

def struct_xml_to_man(struct_name, struct_data, out_dir):
    """Generates a manpage for a structure."""
    fname = safe_name(struct_name)

    print(f" GEN {fname}.3")

    content = [
        f'.TH "{fname.upper()}" 3',
        f'.SH NAME',
        f'{struct_name} \\- {struct_data["description"] or "Structure definition"}',
        f'.SH SYNOPSIS',
        f'\\fBstruct {struct_name}\\fR',
        f'.SH MEMBERS'
    ]

    if struct_data["members"]:
        for m_type, m_name, m_args, m_desc in struct_data["members"]:
            content.append('.TP')
            content.append(f'\\fB{m_type} {m_name}{m_args}\\fR')
            content.append(m_desc or "No description available.")
    else:
        content.append("No public members.")

    with open(os.path.join(out_dir, f"{fname}.3"), "w", encoding="utf-8") as f:
        f.write("\n".join(content))


def collect_types(xml_dir):
    '''Collects all enums and structures from all xml files.'''
    all_enums = {}
    all_structs = {}
    skip_enums = {'gp_pixel_type'}

    print(f"Scanning enums and structs in: {xml_dir}")

    for xml_file in glob.glob(os.path.join(xml_dir, "*.xml")):
        try:
            tree = ET.parse(xml_file)
            for enum in tree.findall(".//memberdef[@kind='enum']"):
                name = enum.findtext('name')
                if name in skip_enums:
                    continue
                v_list = []
                for v in enum.findall('enumvalue'):
                    v_list.append((v.findtext('name'), get_text(v.find('briefdescription/para'))))
                all_enums[name] = {
                    "description": get_text(enum.find('briefdescription/para')),
                    "items": v_list
                }

            for compound in tree.findall(".//compounddef[@kind='struct']"):
                name = compound.findtext('compoundname')
                members = []
                for mem in compound.findall(".//memberdef[@kind='variable']"):
                    m_type = get_text(mem.find('type'))
                    m_name = mem.findtext('name')
                    m_args = get_text(mem.find('argsstring'))
                    m_desc = get_text(mem.find('briefdescription/para'))
                    members.append((m_type, m_name, m_args, m_desc))
                all_structs[name] = {
                    "description": get_text(compound.find('briefdescription/para')),
                    "members": members
                }
        except:
            continue

    print(f"Found {len(all_enums)} enums.")
    print(f"Found {len(all_structs)} structs.")

    return all_enums, all_structs

def process_all(xml_dir, out_dir):
    if not os.path.isdir(xml_dir):
        print(f"Dir '{xml_dir}' does not exist.")
        return

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    (all_enums, all_structs) = collect_types(xml_dir)

    for s_name, s_data in all_structs.items():
        struct_xml_to_man(s_name, s_data, out_dir)

    for xml_file in glob.glob(os.path.join(xml_dir, "*.xml")):
        if "index.xml" in xml_file or "combine.xml" in xml_file:
            continue
        xml_to_man(xml_file, out_dir, all_enums, all_structs)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: xml2man.py input_dir/ output_dir")
    else:
        process_all(sys.argv[1], sys.argv[2])
