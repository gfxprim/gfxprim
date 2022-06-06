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

sub convert_font
{
	my ($font, $name, $bold) = @_;
	my $glyphs = $font->{"glyphs"};
	my @offsets = ();
	my $offset = 0;
	my $max_width = 0;
	my $max_advance = 0;

	my $font_id = $bold ? "font_bold" : "font";

	print("static uint8_t ${font_id}_glyphs[] = {\n");

	for (my $i = 32; $i <= 126; $i++) {
		my $glyph = $glyphs->[$i];

		$max_width = max($max_width, $glyph->{'w'});
		$max_advance = max($max_advance, $glyph->{'x_advance'});
	}

	for (my $i = 32; $i <= 126; $i++) {
		my $glyph = $glyphs->[$i];

		$glyph = embold($glyph, $max_width) if $bold;

		die "missing glyph $i" if !$glyph;

		push(@offsets, $offset);

		printf("\t/* '%c' */\t %2i, %2i, %2i, %2i, %2i,\n", $i,
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
		printf("0x%04x,", $offsets[$i]);
		print(" ") if ($i % 8 != 7);
	}
	print("\n};\n\n");


	print("static struct gp_font_face $font_id = {\n");
	print("\t.family_name = \"$name\",\n");

	if ($bold) {
		print("\t.style = GP_FONT_MONO | GP_FONT_BOLD,\n");
	} else {
		print("\t.style = GP_FONT_MONO,\n");
	}

	printf("\t.ascend = %i,\n", $font->{'ascend'});
	printf("\t.descend = %i,\n", $font->{'descent'});
	print("\t.max_glyph_width = $max_width,\n");
	print("\t.max_glyph_advance = $max_advance,\n");
	print("\t.glyph_tables = 1,\n");
	print("\t.glyphs = {\n");
	print("\t\t{\n");
	print("\t\t\t.glyphs = ${font_id}_glyphs,\n");
	print("\t\t\t.offsets = ${font_id}_offsets,\n");
	print("\t\t\t.min_glyph = 0x20,\n");
	print("\t\t\t.max_glyph = 0x7f,\n");
	print("\t\t}\n");
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
print("\t.fonts = {\n");
print("\t\t&font,\n");
print("\t\t&font_bold,\n");
print("\t\tNULL\n");
print("\t}\n");
print("};\n");
