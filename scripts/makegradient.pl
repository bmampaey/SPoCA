#!/usr/bin/perl
#
# File: makegradient.pl
# Written by Benjamin Mampaey on 4 Feb 2010
# This script will generate a gradient

use strict;

my $numberColors = 0;
my $maxNumberColors = 0;
my $maxL = 50;
my $width = 20;
my @S = (100, 50);
my @L = (100, 75, 87.5, 62.5, 93.75, 68.75, 81.25);
my @H = (0, 60, 120, 180, 240, 300, 30, 90, 150, 210, 270, 330, 15, 75, 135, 195, 255, 315, 45, 105, 165, 225, 285, 345); 
my @P = (0, 180);
my $prefix = "hsb";
my $filename = "gradient.png";
my $c = 0;
my $p = 0;
my $colors;
my $preview;

#We read the arguments
use Getopt::Std;
my %opt;
my $opt_string = 'n:l:o:cph';
getopts( "$opt_string", \%opt ) or usage();
if( $opt{h} )
{
	print "Create a gradient of color in hsb format with Imagemagick, or to include in C.\nThe gradient allways start with the black color.\n";
	print "Options :\n";
	print "\t-h : This help and exit.\n";
	print "\t-c : Gradient for C.\n";
	print "\t-o : Filename for output.\n";
	print "\t-n : Number of colors in the gradient (all of them if 0 or not specified).\n";
	print "\t-l : Maximal lightness (between 50 and 100) for primary colors (red, blue, green). 100 if you want primary colors.\n";
	print "\t-p : Generate a preview (gradientpreview.png).\n";
	
	exit 0;
}
if( $opt{n} )
{
	$maxNumberColors = $opt{n};
}
if( $opt{l} )
{
	$maxL = $opt{l};
}
if( $opt{c} )
{
	$c = 1;
}
if( $opt{o} )
{
	$filename = $opt{o};
}
elsif ($c)
{
	$filename = "gradient.h";
}
if( $opt{p} )
{
	$p = 1;
}

# We create the black

if($c)
{
	
	$colors = "\"0, 0, 0\"";
}
else #ImageMagick
{
	$colors = "\"xc:" . $prefix . "(0, 0%, 0%)\"";
}
if($p)
{
	$preview = "-size 150x20 -background \"" . $prefix . "(0, 0%, 0%)\" -gravity center -fill white -pointsize 12 label:\"" . $prefix . "(0, 0%, 0%)\"";
}

# We add the colors

COLOR : while (1)
{
	foreach my $s (@S)
	{
		foreach my $l (@L)
		{
			HUE : foreach my $h (@H)
			{
				foreach my $p (@P)
				{
	 
					if ($h > $p - $width && $h <= $p && $l > (((100 - $maxL)/$width) * ($h - $p)) + $maxL) #Too close of a primary color
					{
						next HUE;
					}
					if ($h < $p + $width && $h > $p && $l > (((100 - $maxL)/$width) * ($p - $h)) + $maxL) #Too close of a primary color
					{
						next HUE;
					}
				}

				if($c)
				{
					$colors .= ",\"" . sprintf("%.2f", ($h/360)) . ", " . sprintf("%.2f", ($s/100)) . ", " . sprintf("%.2f", ($l/100)) . "\"";
				}
				else #ImageMagick
				{
					$colors .= " \"xc:" . $prefix . "($h, $s%, $l%)\"";
				}
				if($p)
				{
					$preview .= " -size 150x20 -background \"" . $prefix . "($h, $s%, $l%)\" -gravity center -fill black -pointsize 12 label:\"" . $prefix . "($h, $s%, $l%)\"";
				}
				++$numberColors;
				last COLOR if($maxNumberColors != 0 && $numberColors >= $maxNumberColors);
						
			}
		}
	}
	
	last COLOR if($maxNumberColors == 0);
}

if($c)
{
	open FILE, ">$filename" or die "Could'nt create $filename = $!";
	print FILE "#ifndef GRADIENT\n";
	print FILE "#define GRADIENT\n";
	print FILE "const unsigned gradientMax = $numberColors;\n";
	print FILE "const char* gradient[] = {" . $colors . "};\n";
	print FILE "#endif\n";
	close FILE; 
}
else #ImageMagick
{
	print STDERR `convert -size 1x1 $colors -append $filename`."\n";
}
if($p)
{
	print STDERR `convert $preview -append gradientpreview.png`."\n";
}
print STDERR "Created a gradient of $numberColors colors.\n";

