#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
#
# Generate simple case-folding lookup tables (uppercase, lowercase, titlecase)
# and the "is letter" / punctuation class range tables from UnicodeData.txt
# the "is space" range table from PropList.txt and the line break class range
# table from LineBreak.txt.
#
# The format is described in:
#   https://www.unicode.org/reports/tr44/#UnicodeData.txt
#   https://www.unicode.org/reports/tr44/#PropList.txt
#   https://www.unicode.org/reports/tr14/
#
# We use fields:
#   [0]  codepoint               (hex, no prefix)
#   [2]  general category        (Lu/Ll/Lt/Lm/Lo/...)
#   [12] simple uppercase mapping
#   [13] simple lowercase mapping
#   [14] simple titlecase mapping
#
# Only simple (1:1) case mappings are emitted.  Complex mappings such as
# German "sharp s" -> "SS" (which expand to multiple codepoints) live in
# SpecialCasing.txt and are out of scope here.

import sys

def parse_unicode_data(path):
    upper = []         # (cp, upper_cp)
    lower = []         # (cp, lower_cp)
    title = []         # (cp, title_cp) where title != upper
    letter_ranges = [] # list of (start, end) inclusive
    punct_ranges = []  # list of (start, end, punct class) inclusive

    # Every general category that starts with a P, mapped to the enum
    # gp_utf_punct member it is emitted as.  Which of them a caller wants is
    # its own business -- CSS 2.1 5.12.2 ::first-letter for example names
    # Ps, Pe, Pi, Pf and Po but neither dashes nor connectors.
    punct_cats = {
        "Pc": "GP_UTF_PUNCT_CONNECTOR",
        "Pd": "GP_UTF_PUNCT_DASH",
        "Ps": "GP_UTF_PUNCT_OPEN",
        "Pe": "GP_UTF_PUNCT_CLOSE",
        "Pi": "GP_UTF_PUNCT_INIT_QUOTE",
        "Pf": "GP_UTF_PUNCT_FINAL_QUOTE",
        "Po": "GP_UTF_PUNCT_OTHER",
    }

    # Large contiguous blocks (CJK ideographs, Hangul syllables, etc.) are
    # encoded as a pair of marker lines: <..., First> at the range start and
    # <..., Last> at the range end.  We expand them into a single range.
    range_start = None
    range_category = None

    for line in open(path):
        fields = line.rstrip("\n").split(";")
        if len(fields) < 15:
            continue

        cp = int(fields[0], 16)
        name = fields[1]
        category = fields[2]
        upper_s = fields[12]
        lower_s = fields[13]
        title_s = fields[14]

        # Handle range markers.  Both First and Last carry the same category;
        # if it's a letter, emit the whole inclusive range.
        if name.endswith(", First>"):
            range_start = cp
            range_category = category
            continue
        if name.endswith(", Last>"):
            if range_start is not None and range_category:
                if range_category.startswith("L"):
                    letter_ranges.append((range_start, cp))
                if range_category in punct_cats:
                    punct_ranges.append((range_start, cp,
                                         punct_cats[range_category]))
            range_start = None
            range_category = None
            continue

        if category.startswith("L"):
            letter_ranges.append((cp, cp))

        if category in punct_cats:
            punct_ranges.append((cp, cp, punct_cats[category]))

        if upper_s:
            upper.append((cp, int(upper_s, 16)))
        if lower_s:
            lower.append((cp, int(lower_s, 16)))
        if title_s:
            t = int(title_s, 16)
            # Titlecase usually equals uppercase; only store when it differs
            # so the runtime can fall back to gp_utf_toupper().
            if upper_s and t == int(upper_s, 16):
                pass
            else:
                title.append((cp, t))

    return upper, lower, title, letter_ranges, punct_ranges


def parse_prop_list(path, prop):
    """Returns (start, end) inclusive ranges with the binary property prop."""
    ranges = []

    for line in open(path):
        line = line.split("#")[0].strip()
        if not line:
            continue

        cps, name = [f.strip() for f in line.split(";")]
        if name != prop:
            continue

        start, _, end = cps.partition("..")
        ranges.append((int(start, 16), int(end or start, 16)))

    return ranges


def parse_marks(path):
    """Returns the set of the codepoints with general category Mn or Mc."""
    marks = set()

    for line in open(path):
        fields = line.split(";")
        if len(fields) > 2 and fields[2] in ("Mn", "Mc"):
            marks.add(int(fields[0], 16))

    return marks


# The UAX #14 line break classes stored in the table, emitted as enum
# gp_utf_lb. The classes that gp_utf_break() does not tell apart are merged,
# either by the rule LB1 or because no rule distinguishes them.
brk_classes = [
    "AL", "BK", "CR", "LF", "CM", "WJ", "ZW", "GL", "SP", "ZWJ", "B2", "BA",
    "BB", "HY", "CB", "CL", "CP", "EX", "NS", "OP", "QU", "IS", "NU", "PO",
    "PR", "SY", "AK", "AP", "AS", "EB", "EM", "H2", "H3", "HL", "ID", "JL",
    "JV", "JT", "RI", "VF", "VI",
]

brk_merge = {
    "AI": "AL",
    "SA": "AL",
    "SG": "AL",
    "XX": "AL",
    "NL": "BK",
    "HH": "BA",
    "IN": "NS",
    "CJ": "NS",
}


def brk_class(cls):
    cls = brk_merge.get(cls, cls)

    if cls not in brk_classes:
        sys.exit("Unknown line break class " + cls)

    return cls


def parse_line_break(path, marks):
    """Returns (start, class) pairs that cover all codepoints, sorted.

    Each class extends up to the next start, codepoints that are not listed
    are XX, the LineBreak.txt default. The classes are merged by brk_merge.

    The SA marks are resolved to CM as in the UAX #14 rule LB1, so that they
    are kept with their base characters."""
    ranges = []

    for line in open(path):
        line = line.split("#")[0].strip()
        if not line:
            continue

        cps, cls = [f.strip() for f in line.split(";")]
        first, _, last = cps.partition("..")
        start = int(first, 16)
        end = int(last or first, 16)

        if cls == "SA":
            for cp in range(start, end + 1):
                ranges.append((cp, cp, "CM" if cp in marks else cls))
        else:
            ranges.append((start, end, cls))

    out = []
    next_cp = 0

    for start, end, cls in sorted(ranges):
        if start > next_cp and (not out or out[-1][1] != brk_class("XX")):
            out.append((next_cp, brk_class("XX")))
        if not out or out[-1][1] != brk_class(cls):
            out.append((start, brk_class(cls)))
        next_cp = end + 1

    if next_cp <= 0x10ffff and out[-1][1] != brk_class("XX"):
        out.append((next_cp, brk_class("XX")))

    return out


def collapse_ranges(ranges):
    """Merge a list of (start, end[, class]) inclusive ranges into a sorted,
    coalesced list.  A range that carries a class merges only into a
    neighbour of the very same class, so the class survives the collapse."""
    if not ranges:
        return []
    ranges = sorted(ranges)
    out = [ranges[0]]
    for r in ranges[1:]:
        last = out[-1]
        if r[0] <= last[1] + 1 and r[2:] == last[2:]:
            out[-1] = (last[0], max(last[1], r[1])) + last[2:]
        else:
            out.append(r)
    return out


def emit_map(name, entries):
    entries = sorted(entries)
    print("static const struct gp_utf_case_entry {} [] = {{".format(name))
    for cp, m in entries:
        print("\t{{0x{:04x}, 0x{:04x}}},".format(cp, m))
    print("};")
    print("#define {}_LEN GP_ARRAY_SIZE({})".format(name.upper(), name))
    print()


def emit_ranges(name, ranges):
    print("static const struct gp_utf_range {} [] = {{".format(name))
    for start, end in ranges:
        print("\t{{0x{:04x}, 0x{:04x}}},".format(start, end))
    print("};")
    print("#define {}_LEN GP_ARRAY_SIZE({})".format(name.upper(), name))
    print()


# The line break table layout, the classes below BRK_BLOCK_LIMIT are stored
# in blocks of 2^BRK_BLOCK_BITS codepoints, each class change is a 16bit
# entry with an offset into the block and the class.
BRK_BLOCK_BITS = 10
BRK_CLASS_BITS = 16 - BRK_BLOCK_BITS
BRK_BLOCK_LIMIT = 0x40000

# The Hangul syllables are LV (H2) when they have no final consonant and LVT
# (H3) otherwise, which is computed instead of stored.
HANGUL_FIRST = 0xac00
HANGUL_LAST = 0xd7a3
HANGUL_T_COUNT = 28


def brk_hangul(ranges):
    """Checks the Hangul syllable classes and replaces them with H3."""
    starts = [start for start, _ in ranges]
    out = []

    for i, (start, cls) in enumerate(ranges):
        end = starts[i + 1] - 1 if i + 1 < len(starts) else 0x10ffff

        for cp in range(max(start, HANGUL_FIRST), min(end, HANGUL_LAST) + 1):
            exp = "H2" if (cp - HANGUL_FIRST) % HANGUL_T_COUNT == 0 else "H3"
            if cls != exp:
                sys.exit("Hangul syllable 0x{:04x} is {} not {}".format(cp, cls, exp))

        if start >= HANGUL_FIRST and end <= HANGUL_LAST:
            cls = "H3"

        if not out or out[-1][1] != cls:
            out.append((start, cls))

    return out


def emit_lb_enum():
    print("/*")
    print(" * The UAX #14 line break classes used by gp_utf_break(), the classes")
    print(" * it does not tell apart are merged.")
    print(" */")
    print("enum gp_utf_lb {")
    for cls in brk_classes:
        merged = sorted(k for k, v in brk_merge.items() if v == cls)
        if merged:
            print("\t/* Also {} */".format(", ".join(merged)))
        print("\tGP_UTF_LB_{},".format(cls))
    print("\tGP_UTF_LB_CNT,")
    print("};")
    print()


def emit_brk_tables(ranges):
    ranges = brk_hangul(ranges)
    block_size = 1 << BRK_BLOCK_BITS
    starts = [start for start, _ in ranges]
    index = []
    entries = []
    r = 0

    for block in range(0, BRK_BLOCK_LIMIT, block_size):
        # The range the block starts in
        while r + 1 < len(ranges) and starts[r + 1] <= block:
            r += 1

        index.append(len(entries))
        entries.append((0, ranges[r][1]))

        i = r + 1
        while i < len(ranges) and starts[i] < block + block_size:
            entries.append((starts[i] - block, ranges[i][1]))
            i += 1

    index.append(len(entries))

    if len(entries) > 0xffff:
        sys.exit("Too many line break table entries")

    print("#define GP_UTF_LB_BLOCK_BITS {}".format(BRK_BLOCK_BITS))
    print("#define GP_UTF_LB_CLASS_BITS {}".format(BRK_CLASS_BITS))
    print("#define GP_UTF_LB_BLOCK_LIMIT 0x{:05x}".format(BRK_BLOCK_LIMIT))
    print("#define GP_UTF_LB_HANGUL_FIRST 0x{:04x}".format(HANGUL_FIRST))
    print("#define GP_UTF_LB_HANGUL_LAST 0x{:04x}".format(HANGUL_LAST))
    print("#define GP_UTF_LB_HANGUL_T_COUNT {}".format(HANGUL_T_COUNT))
    print()
    print("/* The first entry of each block in gp_utf_lb_entries */")
    print("static const uint16_t gp_utf_lb_index[] = {")
    for i in range(0, len(index), 8):
        print("\t" + " ".join("{},".format(v) for v in index[i:i + 8]))
    print("};")
    print()
    print("/* Class changes in a block, the offset in the block and the class */")
    print("static const uint16_t gp_utf_lb_entries[] = {")
    for block in range(len(index) - 1):
        first = block << BRK_BLOCK_BITS
        if block:
            print()
        print("\t/* Block {} 0x{:05x}..0x{:05x} starts at entry {} */".format(
              block, first, first + block_size - 1, index[block]))
        for off, cls in entries[index[block]:index[block + 1]]:
            print("\t0x{:03x} << GP_UTF_LB_CLASS_BITS | GP_UTF_LB_{},".format(off, cls))
    print("};")
    print()
    print("/* The classes from GP_UTF_LB_BLOCK_LIMIT up */")
    print("static const struct gp_utf_lb_range gp_utf_lb_ranges[] = {")
    r = max(i for i in range(len(ranges)) if starts[i] <= BRK_BLOCK_LIMIT)
    print("\t{{0x{:05x}, GP_UTF_LB_{}}},".format(BRK_BLOCK_LIMIT, ranges[r][1]))
    for start, cls in ranges[r + 1:]:
        print("\t{{0x{:05x}, GP_UTF_LB_{}}},".format(start, cls))
    print("};")
    print("#define GP_UTF_LB_RANGES_LEN GP_ARRAY_SIZE(gp_utf_lb_ranges)")
    print()


def emit_punct_ranges(name, ranges):
    print("static const struct gp_utf_punct_range {} [] = {{".format(name))
    for start, end, cls in ranges:
        print("\t{{0x{:04x}, 0x{:04x}, {}}},".format(start, end, cls))
    print("};")
    print("#define {}_LEN GP_ARRAY_SIZE({})".format(name.upper(), name))
    print()


def main():
    if len(sys.argv) != 4:
        print("Usage: gen_utf_tables.py UnicodeData.txt PropList.txt LineBreak.txt",
              file=sys.stderr)
        sys.exit(1)

    upper, lower, title, letter_ranges, punct_ranges = parse_unicode_data(sys.argv[1])
    space_ranges = parse_prop_list(sys.argv[2], "White_Space")
    brk_ranges = parse_line_break(sys.argv[3], parse_marks(sys.argv[1]))

    print("// SPDX-License-Identifier: LGPL-2.1-or-later")
    print("/*")
    print(" * Generated by gen_utf_tables.py")
    print(" *")
    print(" * DO NOT MODIFY THIS FILE DIRECTLY!")
    print(" */")
    print()
    print("#ifndef GP_UTF_CASE_H")
    print("#define GP_UTF_CASE_H")
    print()
    print("#include <stdint.h>")
    print("#include <core/gp_common.h>")
    print("#include <utils/gp_utf.h>")
    print()
    print("struct gp_utf_case_entry {")
    print("\tuint32_t cp;")
    print("\tuint32_t mapped;")
    print("};")
    print()
    print("struct gp_utf_range {")
    print("\tuint32_t start;")
    print("\tuint32_t end;")
    print("};")
    print()
    print("struct gp_utf_punct_range {")
    print("\tuint32_t start;")
    print("\tuint32_t end;")
    print("\tenum gp_utf_punct type;")
    print("};")
    print()
    emit_lb_enum()
    print("/* A class from start up to the start of the next range */")
    print("struct gp_utf_lb_range {")
    print("\tuint32_t start:24;")
    print("\tuint32_t type:8;")
    print("};")
    print()

    emit_map("gp_utf_upper_map", upper)
    emit_map("gp_utf_lower_map", lower)
    emit_map("gp_utf_title_map", title)
    emit_ranges("gp_utf_letter_ranges", collapse_ranges(letter_ranges))
    emit_punct_ranges("gp_utf_punct_ranges", collapse_ranges(punct_ranges))
    emit_ranges("gp_utf_space_ranges", collapse_ranges(space_ranges))
    emit_brk_tables(brk_ranges)

    print("#endif /* GP_UTF_CASE_H */")


if __name__ == "__main__":
    main()
