#!/usr/bin/perl
#
# File: makeset.pl
# Written by Benjamin Mampaey on 24 June 2009
# This script will help making sets for SPoCA or other instruments


#use strict;
use File::Basename;
use File::Spec::Functions;
use Data::Dumper;
use Cwd;
use POSIX;
use Time::Local;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :constants );
use List::Util qw[min max];

my $all = 0;
my $yes = 0;
my $answer = "no";

my $repository = getcwd();
my $wavelengthlist;
my @filenames;

sub parse_arguments
{
	my $help = undef;
	if (@ARGV == 0)
	{
		$help = 1;
	}
	else
	{
		use Getopt::Long;
		GetOptions(
	            "r:s"=>\$repository,
	            "w:s"=>\$wavelengthlist,
	            "h"=>\$help,
		);
	}

	if (! defined $wavelengthlist)
	{
		$help = 1;		
	}

	if (! defined $repository)
	{
		$help = 1;		
	}

	@filenames = @ARGV or $help = 1;
	
	if ($help == 1)
	{
		print STDERR "Usage: makeset.pl -w wavelenght1,wavelengt2 [-r repository_path] image1.fits image2.fits ... \n";
		print STDERR "This script creates a dataset, creating numeroted tuples of soft links to the filenames provided\n";
		print STDERR "The repository where the soft links will be created can be specified using the -r option, by default it is the current working directory\n";
		exit(2);

	}
}

parse_arguments();

my @wavelengths = split /,/,$wavelengthlist;

# We sort the filenames on the wavelength


my @files0;
my @files1;

 
foreach my $filename (@filenames)
{
	my $status;
	my $fitsfile;
	fits_open_image($fitsfile, $filename, READONLY,$status);
	if($status)
	{
		warn "error opening file $filename";
		next;
	}

	my ($hash_ref, $status) = $fitsfile->read_header;
	if($status)
	{
		warn "error reading header from file $filename";
		next;
	}

	my $date_obs = $hash_ref->{'DATE-OBS'};
	if (! $date_obs)
	{
		warn "Missing keyword DATE-OBS in file $filename";
		next;
		
	}
	my $wavelnth = $hash_ref->{'WAVELNTH'};
	if (! $wavelnth )
	{
		warn "Missing keyword WAVELNTH in file $filename";
        	next;
        	
	}
	if ($wavelnth == @wavelengths[0])
	{
		$date_obs =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)# or die "DATE-OBS of file $filename is incorrect : $date_obs";
		my $date = timegm($6, $5, $4, $3, $2-1, $1) or die "date of file $filename is incorrect : $date_obs";
		push @files0, [$date, $filename];
	}
	if ($wavelnth == @wavelengths[1])
	{
		$date_obs =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)# or die "DATE-OBS of file $filename is incorrect : $date_obs";
		my $date = timegm($6, $5, $4, $3, $2-1, $1) or die "date of file $filename is incorrect : $date_obs";
		push @files1, [$date, $filename];
	}
}

#print "files0 ", Dumper @files0;
#print "files1 ", Dumper @files1;

# We sort according to time

@files0 = sort { $a->[0] <=> $b->[0] } @files0;
@files1 = sort { $a->[0] <=> $b->[0] } @files1;

#print "sorted files0 ", Dumper @files0;
#print "sorted files1 ", Dumper @files1;

my $precision = max (ceil(log(@filenames)/log(10))+1, 3);
# print "precision : ", $precision, "\n";

# Now we create the tuples 

my ($index0, $index1) = (0,0);
my $linkindex = 0;
my $all = 0;
my $yes = 0;

while($index0 < @files0 && $index1 < @files1)
{
	if($files0[$index0]->[0] < $files1[$index1]->[0])
	{
		++$index0 while ($index0 < $#files0 and abs($files1[$index1]->[0] - $files0[$index0 + 1]->[0]) < abs($files1[$index1]->[0] - $files0[$index0]->[0]));
	
	}
	else
	{
		++$index1 while ($index1 < $#files1 and abs($files0[$index0]->[0] - $files1[$index1 + 1]->[0]) < abs($files0[$index0]->[0] - $files1[$index1]->[0]));
	}
	#print "tuple $linkindex : ", $files0[$index0]->[1], " ", $files1[$index1]->[1], "\n";
	my $link0 = $repository.'/'.sprintf("%0*d", $precision, $linkindex).'.'.(basename($files0[$index0]->[1]));
	my $link1 = $repository.'/'.sprintf("%0*d", $precision, $linkindex).'.'.(basename($files1[$index1]->[1]));
	if (! $all)
	{
		print "About to creates soft link $link0 and $link1 \nIs it ok ? [yes/no/all/none] :";
		my $answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^a/i) ? 1 : 0;
		exit if ($answer =~ /none/i);
	
	}
	if ($all or $yes)
	{ 
		symlink File::Spec->rel2abs($files0[$index0]->[1]), $link0;
		symlink File::Spec->rel2abs($files1[$index1]->[1]), $link1;
	}
	
	++$linkindex;
	++$index0;
	++$index1;
}




