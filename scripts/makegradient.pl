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
my @P = (0, 120, 240, 360);
my $filename = "gradient";
my $c = 1;
my $p = 1;
my $i = 1;
my @magick_colors;
my @dot_colors;
my $preview;

#We read the arguments
use Getopt::Std;
my %opt;
my $opt_string = 'n:l:o:icph';
getopts( "$opt_string", \%opt ) or usage();
if( $opt{h} )
{
	print "Create a gradient of color in hsb format with Imagemagick, or to include in C.\nThe gradient allways start with the black color.\n";
	print "Options :\n";
	print "\t-h : This help and exit.\n";
	print "\t-i : Make a png gradient usable with imagemagick clut option";
	print "\t-c : Make the corresponding c header file.\n";
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
if( $opt{c} || $opt{i} || $opt{p})
{
	$c = $opt{c} ? 1: 0;
	$i = $opt{i} ? 1: 0; 
	$p = $opt{p} ? 1: 0;
}
if( $opt{o} )
{
	$filename = $opt{o};
}


# We create the black

push @dot_colors, '[color=\"0, 0, 0\"];';
push @magick_colors, 'hsb(0%, 0%, 0%)';



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

				push @dot_colors, '[color=\"'.sprintf("%.2f", ($h/360)) . ", " . sprintf("%.2f", ($s/100)) . ", " . sprintf("%.2f", ($l/100)) . '\"];';
				push @magick_colors, "hsb(".sprintf("%.2f", ((100*$h)/360))."%, $s%, $l%)";
				++$numberColors;
				last COLOR if($maxNumberColors != 0 && $numberColors >= $maxNumberColors);
						
			}
		}
	}
	
	last COLOR if($maxNumberColors == 0);
}


if($c)
{
	my $magick_string = join '", "', @magick_colors;
	$magick_string = '"'.$magick_string.'"';
	my $dot_string = join '", "', @dot_colors;
	$dot_string = '"'.$dot_string.'"';
	my $cpp_filename = $filename.".cpp";
	open FILE, ">$cpp_filename" or die "Could'nt create $cpp_filename : $!";
	print FILE "#include \"gradient.h\"\n";
	print FILE "const unsigned gradientMax = $numberColors;\n";
	print FILE "const char* dot_gradient[] = {$dot_string};\n";
	print FILE "const char* magick_gradient[] = {$magick_string};\n";
	close FILE; 
	my $h_filename = $filename.".h";
	open FILE, ">$h_filename" or die "Could'nt create $h_filename : $!";
	print FILE "#ifndef GRADIENT_H\n";
	print FILE "#define GRADIENT_H\n";
	print FILE "extern const unsigned gradientMax;\n";
	print FILE "extern const char* dot_gradient[];\n";
	print FILE "extern const char* magick_gradient[];\n";
	print FILE "#endif\n";
	close FILE; 

}
if($i) #ImageMagick
{
	my $magick_string = join '" "xc:', @magick_colors;
	$magick_string = '"xc:' . $magick_string . '"';
	my $i_filename = $filename.".png";
	print STDERR `convert -size 1x1 $magick_string -append $i_filename`."\n";
}
if($p)
{
	my $magick_string = shift @magick_colors;
	$magick_string = ' -size 150x20 -background "'.$magick_string.'" -gravity center -fill white -pointsize 12 label:"'.$magick_string.'"';
	$magick_string.= ' -size 150x20 -background "'.$_.'" -gravity center -fill black -pointsize 12 label:"'.$_.'"' foreach(@magick_colors);
	my $p_filename = $filename."_preview.png";
	print STDERR `convert $magick_string -append $p_filename`."\n";
}
print STDERR "Created a gradient of $numberColors colors.\n";

