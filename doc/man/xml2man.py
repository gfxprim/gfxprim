#!/usr/bin/env python3
#
# SPDX-License-Identifier: GPL-2.0-or-later

import xml.etree.ElementTree as ET
import sys, os, glob, traceback
from datetime import date

def get_text(node):
    """Extracts text from leaf xml element."""
    if node is None:
        return ""
    return "".join(node.itertext()).strip().replace('\n', ' ')

def type_space(c_type):
    """Returns a space or empty string to padd c_type."""
    if c_type.endswith('*'):
        return ''
    return ' '

def get_params(param_list, params_type, is_macro, name, all_enums, all_structs, see_also):
    """Extracts parameters from a parameterlist XML node and formats them for manpages."""
    params = []
    if param_list is None:
        return params

    try:
        items = param_list.findall('.//*parameteritem') if hasattr(param_list, 'tag') and '}' in param_list.tag else param_list.findall('.//parameteritem')
    except Exception as e:
        print(f"ERROR: Failed to search for <parameteritem> inside <parameterlist> for function '{name}': {e}")
        traceback.print_exc()
        return params

    for item in items:
        p_name = "unknown"
        try:
            name_node = item.find('.//*parametername') or item.find('.//parametername')
            desc_node = item.find('.//*para') or item.find('.//parameterdescription/para')

            if name_node is None:
                continue

            p_name = get_text(name_node)
            p_desc = get_text(desc_node) if desc_node is not None else "No description."
            p_type = ''

            if p_name in params_type:
                p_type = params_type[p_name]
            else:
                if not is_macro and p_name != '...':
                    print(f"ERROR: param {p_name} not in the function {name} argument list!")

            params.append(f'.TP\n\\fB{p_type}{type_space(p_type)}\\fI{p_name}\\fR\n{p_desc}')
            p_ctype = clear_type(p_type)

            if all_structs and p_ctype in all_structs:
                if see_also is not None:
                    see_also.append(p_ctype)
                else:
                    print(f"WARNING: 'see_also' list is None, cannot append structure link for '{p_ctype}'")

            if all_enums and p_ctype in all_enums:
                enum_data = all_enums[p_ctype]
                if 'description' not in enum_data or 'items' not in enum_data:
                    raise KeyError(f"Missing 'description' or 'items' in all_enums data for '{p_ctype}'")

                params.append('.sp')
                params.append('.RS 4')
                params.append(enum_data['description'] + "\n")
                for v_name, v_desc in enum_data['items']:
                    params.append(f'.IP "\\fB{v_name}\\fR" 15\n{v_desc or "No description."}')
                params.append('.RE')

        except KeyError as ke:
            print(f"ERROR: Invalid dictionary structure inside all_enums/all_structs when processing parameter '{p_name}' in function '{name}': {ke}")
            traceback.print_exc()
        except TypeError as te:
            print(f"ERROR: Type mismatch (e.g. invalid XML node or non-iterable enum items) for parameter '{p_name}' in function '{name}': {te}")
            traceback.print_exc()
        except Exception as e:
            print(f"ERROR: Unexpected error while processing parameter '{p_name}' in function '{name}': {e}")
            traceback.print_exc()

    return params

def get_desc(node, skip_params=False, params_type=None, is_macro=False, name="", all_enums=None, all_structs=None, see_also=None):
    """Extract description, formats C code, skips a list of parameters. Returns a list of paragraphs."""
    if node is None:
        return []

    if params_type is None: params_type = {}
    if all_enums is None: all_enums = {}
    if all_structs is None: all_structs = {}
    if see_also is None: see_also = []

    paragraphs = []
    current_para = ""

    try:
        if node.text:
            current_para += node.text.replace('\n', ' ')
    except Exception as e:
        print(f"ERROR: Failed to read text from node '{node.tag if hasattr(node, 'tag') else 'unknown'}' in function '{name}': {e}")

    for child in node:
        try:
            tag = child.tag.split('}')[-1] if '}' in child.tag else child.tag

            if tag == 'parameterlist':
                if not skip_params:
                    paragraphs.extend(get_params(child, params_type, is_macro, name, all_enums, all_structs, see_also))
                continue

            if tag == 'simplesect' and child.get('kind') in ['return']:
                continue

            if tag == 'simplesect' and child.get('kind') in ['important', 'warning']:
                current_para += f"\\fI{get_text(child)}\\fR"
                if current_para.strip():
                    paragraphs.append(current_para.strip(' '))
                    current_para = ""
                paragraphs.append(".PP")

            elif tag in ['computeroutput', 'programlisting', 'verbatim']:
                code_parts = []

                def extract_code(n):
                    try:
                        if n.text:
                            code_parts.append(n.text)
                        for sub in n:
                            sub_tag = sub.tag.split('}')[-1] if '}' in sub.tag else sub.tag
                            if sub_tag == 'sp':
                                code_parts.append(' ')
                            extract_code(sub)
                            if sub.tail:
                                code_parts.append(sub.tail)
                    except Exception as ce:
                        print(f"WARNING: Error parsing code block element inside function '{name}': {ce}")

                extract_code(child)
                code_text = ''.join(code_parts)

                if current_para.strip():
                    paragraphs.append(current_para.strip(' '))
                    current_para = ""

                paragraphs.extend([".sp", ".RS 4", ".nf", code_text.strip('\n'), ".fi", ".RE", ".sp"])

            elif tag == 'para':
                if current_para.strip():
                    paragraphs.append(current_para.strip(' '))
                    current_para = ""
                paragraphs.append(".PP")
                paragraphs.extend(get_desc(child, skip_params, params_type, is_macro, name, all_enums, all_structs, see_also))
            else:
                current_para += get_text(child)

            if child.tail:
                current_para += child.tail.replace('\n', ' ')

        except Exception as e:
            child_tag = child.tag if hasattr(child, 'tag') else 'unknown'
            print(f"ERROR: Failed to process child element <{child_tag}> in function '{name}': {e}")
            traceback.print_exc()
            continue

    if current_para.strip():
        paragraphs.append(current_para.strip(' '))

    return [p for p in paragraphs if p]

def get_long_desc(node, skip_params=False, params_type=None, is_macro=False, name="", all_enums=None, all_structs=None, see_also=None):
    """Extracts all long description nodes and returns an array of paragraphs."""
    if node is None:
        return []

    if params_type is None: params_type = {}
    if all_enums is None: all_enums = {}
    if all_structs is None: all_structs = {}
    if see_also is None: see_also = []

    try:
        para_nodes = node.findall('.//*para') if hasattr(node, 'tag') and '}' in node.tag else node.findall('detaileddescription/para')
        if not para_nodes:
            return []

        all_paragraphs = []
        for p in para_nodes:
            desc_list = get_desc(p, skip_params, params_type, is_macro, name, all_enums, all_structs, see_also)
            if desc_list:
                if all_paragraphs:
                    all_paragraphs.append(".PP")
                all_paragraphs.extend(desc_list)

        return all_paragraphs

    except Exception as e:
        print(f"CRITICAL ERROR in get_long_desc for function '{name}': {e}")
        traceback.print_exc()
        return []

def clear_type(p_type):
    """Removes modifiers and keywords from a type e.g. 'enum gp_backend *' -> 'gp_backend'."""
    for word in ['const', 'enum', 'struct', 'volatile']:
        p_type = p_type.replace(word, '')

    return p_type.replace('*', '').replace('&', '').strip()

def safe_name(name):
    return name.replace(':', '_').replace(' ', '_')

def gen_th(name):
    return f'.TH {name} 3 {date.today().isoformat()} "GFXprim manual pages"'

def xml_to_man(xml_path, out_dir, all_enums, all_structs):
    """Iterates over all functions in a xml file and generates man page for each of them."""
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
    except Exception as e:
        print(f"ERROR: Failed to parse XML '{xml_path}': {e}")
        return

    see_also_funcs = [m.findtext('name') for m in tree.findall(".//memberdef[@kind='function']")]
    see_also_macros = [m.findtext('name') for m in root.findall(".//memberdef[@kind='define']")
                       if m.find('param') is not None]

    # Generate docs for functions and macros with parameters
    for func in root.findall(".//memberdef[@kind='function']") + \
                root.findall(".//memberdef[@kind='define']"):
        name = func.findtext('name')
        header_path = func.find('location').get('file')
        header = os.path.basename(os.path.dirname(header_path)) + '/' + os.path.basename(header_path)

        if not name or not ((name.startswith('gp_') or name.startswith('GP_'))) or name.endswith('_'):
            continue

        print(f" GEN {name}.3")

        # SYNOPSIS
        content = [
            f'{gen_th(name)}',
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
                continue
        else:
            type_node = func.find('type')
            ret_type = get_text(type_node) or "void"
            c_ret_type = clear_type(ret_type)

            if c_ret_type in all_structs:
                see_also.append(c_ret_type)

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
            content.append(f"\\fB{ret_type}{type_space(ret_type)}{name}(\\fR{', '.join(params_info)}\\fB);\\fR")
        content.append('.fi')

        # DESCRIPTION
        desc = get_long_desc(func, True, params_type, is_macro, name, all_enums, all_structs, see_also)
        if desc:
            content.append('.SH DESCRIPTION')
            content.extend(desc)

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
                    if not is_macro and p_name != '...':
                        print(f"ERROR: param {p_name} not in the function {name} argument list!")

                content.append(f'.TP\n\\fB{p_type}{type_space(p_type)}\\fI{p_name}\\fR\n{p_desc}')
                p_ctype = clear_type(p_type)
                if p_ctype in all_structs:
                    see_also.append(p_ctype)
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
    try:
        fname = safe_name(struct_name)

        if not fname.startswith('gp_'):
            return

        print(f" GEN {fname}.3")

        synopsis_declaration = [
            f'.SH SYNOPSIS',
            f'.nf',
            f'\\fBstruct {struct_name} {{\\fR'
        ]

        if struct_data["members"]:
            for m_type, m_name, m_args, _, _ in struct_data["members"]:
                synopsis_declaration.append(f'    \\fB{m_type}\\fR{type_space(m_type)}{m_name}{m_args or ""};')

        synopsis_declaration.append('\\fB};\\fR')
        synopsis_declaration.append('.fi')

        content = [
            f'{gen_th(fname)}',
            f'.SH NAME',
            f'{struct_name} \\- {struct_data["description"] or "Structure definition"}',
        ]

        content.extend(synopsis_declaration)

        if struct_data["desc_long"]:
            content.append('.SH DESCRIPTION')
            content.extend([item.strip() for item in struct_data['desc_long'] if item.strip()])

        content.append('.SH MEMBERS')
        if struct_data["members"]:
            for m_type, m_name, m_args, m_desc, m_long_desc in struct_data["members"]:
                content.append('.TP')
                content.append(f'\\fB{m_type}{type_space(m_type)}{m_name}{m_args or ""}\\fR')

                if m_desc or m_long_desc:
                    content.append('.RS 4')

                if m_desc:
                    content.append('.PP')
                    content.append(m_desc.strip())
                    content.append('.PP')

                if m_long_desc:
                    for item in m_long_desc:
                        item_clean = item.strip()
                        if item_clean:
                            content.append(item_clean)

                if m_desc or m_long_desc:
                    content.append('.RE')

                if not m_desc and not m_long_desc:
                    content.append("No description available.")
        else:
            content.append("No public members.")

        clean_content = [line for line in content if line]

        os.makedirs(out_dir, exist_ok=True)

        filepath = os.path.join(out_dir, f"{fname}.3")
        with open(filepath, "w", encoding="utf-8") as f:
            f.write("\n".join(clean_content) + "\n")

    except KeyError as ke:
        print(f"ERROR: Missing expected key {ke} in struct_data for '{struct_name}'!")
        traceback.print_exc()
    except IOError as ioe:
        print(f"ERROR: Failed to write manpage for '{struct_name}' to directory '{out_dir}': {ioe}")
        traceback.print_exc()
    except Exception as e:
        print(f"CRITICAL ERROR while generating manpage for '{struct_name}': {e}")
        traceback.print_exc()

def collect_types(xml_dir):
    """Collects all enums and structures from all xml files."""
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
                    "desc_long": get_long_desc(enum, True),
                    "items": v_list
                }

            for struct in tree.findall(".//compounddef[@kind='struct']"):
                name = struct.findtext('compoundname')
                members = []
                for mem in struct.findall(".//memberdef[@kind='variable']"):
                    m_type = get_text(mem.find('type'))
                    m_name = mem.findtext('name')
                    m_args = get_text(mem.find('argsstring'))
                    m_desc = get_text(mem.find('briefdescription/para'))
                    m_long_desc = get_long_desc(mem, False)
                    members.append((m_type, m_name, m_args, m_desc, m_long_desc))
                all_structs[name] = {
                    "description": get_text(struct.find('briefdescription/para')),
                    "desc_long": get_long_desc(struct, True),
                    "members": members
                }
        except Exception as e:
            print(f"WARNING: Failed to process '{xml_file}' during type collection: {e}")
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
        try:
            struct_xml_to_man(s_name, s_data, out_dir)
        except Exception as err:
            print(f"Failed to generate {s_name}: {err}")

    for xml_file in glob.glob(os.path.join(xml_dir, "*.xml")):
        if "index.xml" in xml_file or "combine.xml" in xml_file:
            continue
        xml_to_man(xml_file, out_dir, all_enums, all_structs)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: xml2man.py input_dir/ output_dir")
    else:
        process_all(sys.argv[1], sys.argv[2])
