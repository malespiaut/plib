#!/usr/bin/perl -w
use strict;

## txfdump.pl - A .txf font file parser and image extractor
##
## Usage: txfdump.pl blahblah.txf
##
## Human readable font metric information will be printed to stdout,
## and a blahblah.pgm file will be written containing the texture
## image from the file.  Note that the resulting image will be
## vertically reflected, owing to the difference between cartesian x/y
## coordinates used by .txf and the screen raster convention of the
## .pgm format (well, and the author's laziness).  Load it up in your
## favorite viewer and flip it if you care.
##
## Copyright (C) 2002 Andrew James Ross
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License version 2 as
## published by the Free Software Foundation.

my $file = shift or die;
open TXF, $file or die;

# Start out assuming intel byte order
# These get used in the getshort/getint functions below
my $shortType = "v";
my $longType = "V";

sub getsbyte  { my $v=getbyte(); if($v>127) { $v-=256; } return $v; }
sub getbyte  { my $val; read TXF, $val, 1 or die; return ord $val; }
sub getshort { my $val; read TXF, $val, 2 or die; return unpack $shortType, $val; }
sub getint   { my $val; read TXF, $val, 4 or die; return unpack $longType, $val; }

# Check the magic number; it is endian-independant
my $magic = sprintf "0x%8.8x", getint();
die "Bad magic number ($magic)" if $magic ne "0x667874ff";

# Read the next four bytes to determine endianness
my $endian = sprintf "0x%8.8x", getint();
if($endian eq "0x12345678") {
} elsif($endian eq "0x78563412") {
    # Network byte order
    $shortType = "n";
    $longType = "N";
} else {
    die "Bad endianness tag ($endian)";
}

# Read the rest of the header
my $format = getint();
my $texwid = getint();
my $texhgt = getint();
my $linehgt = getint();
my $unk = getint();
my $nglyph = getint();

printf "Magic $magic Endian $endian Format 0x%8.8x\n", $format;
print "Texture: ${texwid}x$texhgt Line: $linehgt Glyphs: $nglyph Unk: $unk\n";

# Read each glyph's metadata
for(my $i=0; $i<$nglyph; $i++) {
    print "Glyph $i\n";
    my $c = getshort();
    print "   char: $c ('", chr($c), "')\n";
    print "  width: ", getbyte(), "\n";
    print " height: ", getbyte(), "\n";
    print "   xoff: ", getsbyte(), "\n";
    print "   yoff: ", getsbyte(), "\n";
    print "   step: ", getbyte(), "\n";
    print "    unk: ", getbyte(), "\n";
    print "      X: ", getshort(), "\n";
    print "      Y: ", getshort(), "\n";
}

# The rest of the file is an image.  Write it out as a .pgm file with
# the same name as the font file.

$file =~ s/\.txf$/.pgm/;
open PGM, ">$file" or die;
print PGM "P2\n$texwid $texhgt\n255\n";

if($format == 0) {
    for(my $i=0; $i<($texwid*$texhgt); $i++) { print PGM getbyte(), " "; }
} else {
    for(my $i=0; $i<($texwid*$texhgt/8); $i++) {
	my $byte =  getbyte();
	for(my $j=0; $j<8; $j++) {
	    print PGM ($byte & (1<<$j)) ? "255 " : "0 ";
	}
    }
}

