#!/usr/bin/perl
#SPDX-License-Identifier: GPL-2.1-or-later

#
# Converts BDF font into a gfxprim C font.
#
# Use it as: ./bdf2c.pl /path/to/font.bdf gfxprim-font-name > font_file.c
#

use strict;
use warnings;

sub load_bitmap
{
	my ($file) = @_;
	my @bitmap;
	my $line;
	my $byte;

	do {
		$line = <$file>;
		chomp $line;


		return \@bitmap if $line =~ /ENDCHAR/;

		if (length($line) > 2) {
			do {
				$byte = substr $line, 0, 2, '';
				push(@bitmap, hex($byte));
			} while ($line);
			$line = 1;
		} else {
			push(@bitmap, hex($line));
		}

	} while ($line);

	return \@bitmap;
}

sub load_glyph
{
	my ($file) = @_;
	my %ret;
	my $line;

	while (1) {
		$line = <$file>;
		chomp $line;

		if ($line =~ /ENCODING\s(\d*)/) {
			$ret{"code"} = $1;
		}

		if ($line =~ /DWIDTH\s(\d*)\s(\d*)/) {
			$ret{"x_advance"} = $1;
			$ret{"y_advance"} = $2;
		}

		if ($line =~ /BBX\s(\d*)\s(\d*)\s([\-\d]*)\s([\-\d]*)/) {
			$ret{"w"} = $1;
			$ret{"h"} = $2;
			$ret{"x_off"} = $3;
			$ret{"y_off"} = $4;
		}

		if ($line =~ /BITMAP/) {
			$ret{"bitmap"} = load_bitmap($file);
			return \%ret;
		}
	}
}

sub load_font
{
	my ($path, $ascend) = @_;
	my @glyphs;
	my %font;
	my $run = 1;
	my $line;

	open(my $file, "<$path") or die $!;

	while (1) {
		$line = <$file>;

		last if not $line;

		chomp $line;

		if ($line =~ /STARTCHAR\s(.*)/) {
			my $glyph = load_glyph($file);
			$glyphs[$glyph->{"code"}] = $glyph;
		}

		$font{"ascend"} = $1 if ($line =~ /FONT_ASCENT\s(\d*)/);
		$font{"descent"} = $1 if ($line =~ /FONT_DESCENT\s(\d*)/);
		$font{"registry"} = $1 if ($line =~ /CHARSET_REGISTRY\s"(.*)"/);
		$font{"encoding"} = $1 if ($line =~ /CHARSET_ENCODING\s"(.*)"/);
	}

	$font{"glyphs"} = \@glyphs;

	$font{"ascend"} = $ascend if $ascend;

	return \%font;
}

sub max
{
	my ($a, $b) = @_;
	return $a if $a > $b;
	return $b;
}

sub embold
{
	my ($glyph, $max_w) = @_;
	my @bitmap;
	my $w = $glyph->{'w'};
	my $h = $glyph->{'h'};
	my $overflow_w = 0;
	my $inc_w = 0;
	my %ret = %$glyph;
	my $bytes = ($w+7)>>3;

	# Check if we overflow bitmap width
	# Check if we overflow last byte in line
	for (my $y = 0; $y < $h; $y++) {
		my $last_b = @{$glyph->{'bitmap'}}[$y * $bytes + $bytes - 1];
		my $last_bit_mask = $w % 8 ? 0x100 >> ($w % 8) : 0x01;

		$inc_w = 1 if ($last_b & $last_bit_mask) && $w < $max_w;
		$overflow_w = 1 if $inc_w && $w % 8 == 0;
	}

	for (my $y = 0; $y < $h; $y++) {
		my $prev_b = 0;
		for (my $x = 0; $x < $bytes; $x++) {
			my $cur_b = @{$glyph->{'bitmap'}}[$y * $bytes + $x];

			my $res_b = $cur_b;
			$res_b |= 0xff & ($cur_b >> 1);
			$res_b |= 0xff & ($prev_b << 7);

			push(@bitmap, $res_b);

			$prev_b = $cur_b;
		}

		push(@bitmap, 0xff & ($prev_b) << 7) if $overflow_w;
	}

	$ret{'bitmap'} = \@bitmap;
	$ret{'w'} += 1 if $inc_w;

	return \%ret;
}

sub gen_glyph_table
{
	my ($glyphs, $first, $last, $max_width, $font_id, $embold, $map) = @_;

	my @offsets = ();
	my $offset = 0;

	print("static uint8_t ${font_id}_glyphs[] = {\n");

	for (my $i = $first; $i <= $last; $i++) {
		my $idx = $i;

		$idx = $map->($i) if $map;
		if ($idx < 0) {
			push(@offsets, -1);
			next;
		}

		my $glyph = $glyphs->[$idx];

		if ($map && !$glyph) {
			push(@offsets, -1);
			next;
		} else {
			die "missing glyph $i" if !$glyph;
		}

		push(@offsets, $offset);

		$glyph = embold($glyph, $max_width) if $embold;

		if ($i < 0x7f) {
			printf("\t/* '%c' */", $i);
		} else {
			printf("\t/* 0x%0x '%c%c' */", $i, 0xc0 | ($i>>6), 0x80 | ($i & 0x3f));
		}

		printf("\t %2i, %2i, %2i, %2i, %2i,\n",
		      $glyph->{'w'}, $glyph->{"h"}, $glyph->{"x_off"},
		      $glyph->{"h"} + $glyph->{"y_off"}, $glyph->{'x_advance'});

		$offset += 5;

		my $bitmap = $glyph->{"bitmap"};

		print("\t\t\t ");

		for (@$bitmap) {
			printf(" 0x%02x,", $_);
			$offset += 1;
		}

		# align on 4 byte boundary
		while ($offset % 4) {
			print(" 0x00,");
			$offset += 1;
		}

		print("\n");
	}
	print("\n};\n\n");

	# offsets[len] == glyphs array size
	push(@offsets, $offset);

	print("static gp_glyph_offset ${font_id}_offsets[] = {");
	for (my $i = 0; defined $offsets[$i]; $i++) {
		print("\n\t") if (!($i % 8));
		if ($offsets[$i] == -1) {
			printf("GP_NOGLYPH,");
		} else {
			printf("    0x%04x,", $offsets[$i]);
		}
		print(" ") if ($i % 8 != 7);
	}
	print("\n};\n\n");
}

sub utf_to_iso8859_2
{
	my ($utf) = @_;

	return 0xa1 if $utf == 0x104;
	return 0xa2 if $utf == 0x2d8;
	return 0xa3 if $utf == 0x141;
	return 0xa4 if $utf == 0x0a4;
	return 0xa5 if $utf == 0x13d;
	return 0xa6 if $utf == 0x15a;
	return 0xa7 if $utf == 0x0a7;
	return 0xa8 if $utf == 0x0a8;
	return 0xa9 if $utf == 0x160;
	return 0xaa if $utf == 0x15e;
	return 0xab if $utf == 0x164;
	return 0xac if $utf == 0x179;
	return 0xad if $utf == 0x0ad;
	return 0xae if $utf == 0x17d;
	return 0xaf if $utf == 0x17b;

	return 0xb0 if $utf == 0x0b0;
	return 0xb1 if $utf == 0x105;
	return 0xb2 if $utf == 0x2db;
	return 0xb3 if $utf == 0x142;
	return 0xb4 if $utf == 0x0b4;
	return 0xb5 if $utf == 0x13e;
	return 0xb6 if $utf == 0x15b;
	return 0xb7 if $utf == 0x2c7;
	return 0xb8 if $utf == 0x0b8;
	return 0xb9 if $utf == 0x161;
	return 0xba if $utf == 0x15f;
	return 0xbb if $utf == 0x165;
	return 0xbc if $utf == 0x17a;
	return 0xbd if $utf == 0x2dd;
	return 0xbe if $utf == 0x17e;
	return 0xbf if $utf == 0x17c;

	return 0xc0 if $utf == 0x154;
	return 0xc1 if $utf == 0x0c1;
	return 0xc2 if $utf == 0x0c2;
	return 0xc3 if $utf == 0x102;
	return 0xc4 if $utf == 0x0c4;
	return 0xc5 if $utf == 0x139;
	return 0xc6 if $utf == 0x106;
	return 0xc7 if $utf == 0x0c7;
	return 0xc8 if $utf == 0x10c;
	return 0xc9 if $utf == 0x0c9;
	return 0xca if $utf == 0x118;
	return 0xcb if $utf == 0x0cb;
	return 0xcc if $utf == 0x11a;
	return 0xcd if $utf == 0x0cd;
	return 0xce if $utf == 0x0ce;
	return 0xcf if $utf == 0x10e;

	return 0xd0 if $utf == 0x110;
	return 0xd1 if $utf == 0x143;
	return 0xd2 if $utf == 0x147;
	return 0xd3 if $utf == 0x0d3;
	return 0xd4 if $utf == 0x0d4;
	return 0xd5 if $utf == 0x150;
	return 0xd6 if $utf == 0x0d6;
	return 0xd7 if $utf == 0x0d7;
	return 0xd8 if $utf == 0x158;
	return 0xd9 if $utf == 0x16e;
	return 0xda if $utf == 0x0da;
	return 0xdb if $utf == 0x170;
	return 0xdc if $utf == 0x0dc;
	return 0xdd if $utf == 0x0dd;
	return 0xde if $utf == 0x162;
	return 0xdf if $utf == 0x0df;

	return 0xe0 if $utf == 0x155;
	return 0xe1 if $utf == 0x0e1;
	return 0xe2 if $utf == 0x0e2;
	return 0xe3 if $utf == 0x103;
	return 0xe4 if $utf == 0x0e4;
	return 0xe5 if $utf == 0x13a;
	return 0xe6 if $utf == 0x107;
	return 0xe7 if $utf == 0x0e7;
	return 0xe8 if $utf == 0x10d;
	return 0xe9 if $utf == 0x0e9;
	return 0xea if $utf == 0x119;
	return 0xeb if $utf == 0x0eb;
	return 0xec if $utf == 0x11b;
	return 0xed if $utf == 0x0ed;
	return 0xee if $utf == 0x0ee;
	return 0xef if $utf == 0x10f;

	return 0xf0 if $utf == 0x111;
	return 0xf1 if $utf == 0x144;
	return 0xf2 if $utf == 0x148;
	return 0xf3 if $utf == 0x0f3;
	return 0xf4 if $utf == 0x0f4;
	return 0xf5 if $utf == 0x151;
	return 0xf6 if $utf == 0x0f6;
	return 0xf7 if $utf == 0x0f7;
	return 0xf8 if $utf == 0x159;
	return 0xf9 if $utf == 0x16f;
	return 0xfa if $utf == 0x0fa;
	return 0xfb if $utf == 0x171;
	return 0xfc if $utf == 0x0fc;
	return 0xfd if $utf == 0x0fd;
	return 0xfe if $utf == 0x163;
	return 0xff if $utf == 0x2d9;

	return -1;
}

sub utf_to_utf
{
	my ($utf) = @_;

	return $utf;
}

my $greek = $ENV{'GREEK'};
my $cyrilic = $ENV{'CYRILIC'};
my $katakana = $ENV{'KATAKANA'};
my $hiragana = $ENV{'HIRAGANA'};

sub add_block
{
	my ($min_glyph, $max_glyph, $font_id, $name) = @_;

	print("\t\t{\n");
	print("\t\t\t.glyphs = ${font_id}_${name}_glyphs,\n");
	print("\t\t\t.offsets = ${font_id}_${name}_offsets,\n");
	print("\t\t\t.min_glyph = $min_glyph,\n");
	print("\t\t\t.max_glyph = $max_glyph,\n");
	print("\t\t},\n");
}

my $ucode_blocks;

sub convert_font
{
	my ($font, $name, $bold) = @_;
	my $glyphs = $font->{"glyphs"};
	my $max_width = 0;
	my $max_advance = 0;
	my $font_id = $bold ? "font_bold" : "font";
	my $enc = $font->{'registry'} . "-" . $font->{'encoding'};
	my $last = 0x7f;

	$last = 0xff if ($enc eq "iso8859-2");
	$last = 0x17e if ($enc eq "ISO10646-1");
	for (my $i = 0x20; $i <= $last; $i++) {
		my $glyph = $glyphs->[$i];

		next if !$glyph;

		$max_width = max($max_width, $glyph->{'w'});
		$max_advance = max($max_advance, $glyph->{'x_advance'});
	}

	gen_glyph_table($glyphs, 0x20, 0x7e, $max_width, $font_id, $bold);

	$ucode_blocks = "GP_UCODE_LATIN_BASIC";

	if ($enc eq "iso8859-2") {
		gen_glyph_table($glyphs, 0xa1, 0x17e, $max_width, $font_id . "_latin_ext", $bold, \&utf_to_iso8859_2);
		$ucode_blocks .= " | GP_UCODE_LATIN_SUP | GP_UCODE_LATIN_EXT_A";
	}

	if ($enc eq "ISO10646-1") {
		gen_glyph_table($glyphs, 0xa1, 0x17e, $max_width, $font_id . "_latin_ext", $bold, \&utf_to_utf);
		$ucode_blocks .= " | GP_UCODE_LATIN_SUP | GP_UCODE_LATIN_EXT_A";
	}

	if ($enc eq "ISO10646-1" && $greek) {
		gen_glyph_table($glyphs, 0x384, 0x3ce, $max_width, $font_id . "_greek", $bold, \&utf_to_utf);
		$ucode_blocks .= " | GP_UCODE_GREEK";
	}

	if ($enc eq "ISO10646-1" && $cyrilic) {
		gen_glyph_table($glyphs, 0x400, 0x45f, $max_width, $font_id . "_cyrilic", $bold, \&utf_to_utf);
		$ucode_blocks .= " | GP_UCODE_CYRILIC";
	}

	if ($enc eq "ISO10646-1" && $katakana) {
		gen_glyph_table($glyphs, 0x30a0, 0x30ff, $max_width, $font_id . "_katakana", $bold, \&utf_to_utf);
		$ucode_blocks .= " | GP_UCODE_KATAKANA";
	}

	if ($enc eq "ISO10646-1" && $hiragana) {
		gen_glyph_table($glyphs, 0x3041, 0x3096, $max_width, $font_id . "_hiragana", $bold, \&utf_to_utf);
		$ucode_blocks .= " | GP_UCODE_KATAKANA";
	}

	print("static struct gp_font_face $font_id = {\n");
	print("\t.family_name = \"$name\",\n");

	if ($bold) {
		print("\t.style = GP_FONT_MONO | GP_FONT_BOLD,\n");
	} else {
		print("\t.style = GP_FONT_MONO,\n");
	}

	my $glyph_tables = 1;

	$glyph_tables+=1 if ($enc eq "iso8859-2" || $enc eq "ISO10646-1");
	$glyph_tables+=1 if ($enc eq "ISO10646-1" && $greek);
	$glyph_tables+=1 if ($enc eq "ISO10646-1" && $cyrilic);
	$glyph_tables+=1 if ($enc eq "ISO10646-1" && $katakana);
	$glyph_tables+=1 if ($enc eq "ISO10646-1" && $hiragana);

	printf("\t.ascend = %i,\n", $font->{'ascend'});
	printf("\t.descend = %i,\n", $font->{'descent'});
	print("\t.max_glyph_width = $max_width,\n");
	print("\t.max_glyph_advance = $max_advance,\n");
	print("\t.glyph_tables = $glyph_tables,\n");
	print("\t.glyphs = {\n");
	print("\t\t{\n");
	print("\t\t\t.glyphs = ${font_id}_glyphs,\n");
	print("\t\t\t.offsets = ${font_id}_offsets,\n");
	print("\t\t\t.min_glyph = 0x20,\n");
	print("\t\t\t.max_glyph = 0x7f,\n");
	print("\t\t},\n");
	if ($enc eq "iso8859-2" || $enc eq "ISO10646-1") {
		add_block("0xa1", "0x17e", $font_id, "latin_ext");
	}
	if ($enc eq "ISO10646-1" && $greek) {
		add_block("0x384", "0x3ce", $font_id, "greek");
	}
	if ($enc eq "ISO10646-1" && $cyrilic) {
		add_block("0x400", "0x45f", $font_id, "cyrilic");
	}
	if ($enc eq "ISO10646-1" && $katakana) {
		add_block("0x30a0", "0x30ff", $font_id, "katakana");
	}
	if ($enc eq "ISO10646-1" && $hiragana) {
		add_block("0x3041", "0x3096", $font_id, "hiragana");
	}

	printf("\n\t}\n");
	print("};\n\n");
}

my $font = load_font($ARGV[0], $ARGV[3]);

print("// $ENV{'LICENSE'}\n") if $ENV{'LICENSE'};

printf("/* Generated file, do not touch */\n\n");
printf("#include <text/gp_font.h>\n\n");

convert_font($font, $ARGV[1], 0);
print("\n");
convert_font($font, $ARGV[1] . "_bold", 1);

print("const gp_font_family __attribute__((visibility (\"hidden\"))) font_family_$ARGV[1] = {\n");
print("\t.family_name = \"$ARGV[2]\",\n");
print("\t.ucode_blocks = $ucode_blocks,\n");
print("\t.fonts = {\n");
print("\t\t&font,\n");
print("\t\t&font_bold,\n");
print("\t\tNULL\n");
print("\t}\n");
print("};\n");
