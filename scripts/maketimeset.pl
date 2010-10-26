#!/usr/bin/perl
#
# File: maketimeset.pl
# Written by Benjamin Mampaey on 30 September 2010
# This script will help making sets for SPoCA


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
my $offset = 3600;

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
	            "o:i"=>\$offset,
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
	
	if (! defined $offset)
	{
		$help = 1;		
	}

	@filenames = @ARGV or $help = 1;
	
	if ($help == 1)
	{
		print STDERR "Usage: maketimeset.pl -w wavelenght1,wavelengt2 -o offset_seconds [-r repository_path] image1.fits image2.fits ... \n";
		print STDERR "This script creates a dataset, creating numeroted tuples of soft links to the filenames provided\n";
		print STDERR "The repository where the soft links will be created can be specified using the -r option, by default it is the current working directory\n";
		print STDERR "The offset option specify the approximative time interval between 2 tuples. It is in seconds.\n";
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

print "Finished creating the list of files\n";

# We sort according to time

@files0 = sort { $a->[0] <=> $b->[0] } @files0;
@files1 = sort { $a->[0] <=> $b->[0] } @files1;

print "Finished sorting the list of files\n";

#print "sorted files0 ", Dumper @files0;
#print "sorted files1 ", Dumper @files1;

my $precision = max (ceil(log(@filenames)/log(10))+1, 3);
# print "precision : ", $precision, "\n";

# Now we create the tuples 

my ($index0, $index1) = (0,0);

my $all = 0;
my $yes = 0;
my @tuples0;
my @tuples1;

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
	push @tuples0, $files0[$index0];
	push @tuples1, $files1[$index1];
	++$index0;
	++$index1;
}

@files0 = ();
@files1 = ();

print "Finished creating the tuples of files\n";

my ($index_now, $index_later) = (0,1);
my @good_indexes = (0);
while($index_later < @tuples0)
{
	my $perfect_time = $tuples0[$index_now]->[0] + $offset;
	while($tuples0[$index_later]->[0] < $perfect_time && $index_later + 1 < @tuples0)
	{
		++$index_later;
	}
	my $later_time_diff = $tuples0[$index_later]->[0] - $perfect_time;
	my $ante_later_time_diff = $perfect_time - $tuples0[$index_later-1]->[0];
	if ($index_later-1 > $index_now && $ante_later_time_diff < $later_time_diff)
	{
		--$index_later;
	}
	push @good_indexes, $index_later;
	$index_now = $index_later;
	++$index_later;
}

print "Finished creating the good tuples of files\n";

my $linkindex = 0;
foreach my $good_index (@good_indexes)
{
	print "tuple $linkindex : ", $tuples0[$good_index]->[1], " ", $tuples1[$good_index]->[1], "\n";
	my $link0 = $repository.'/'.sprintf("%0*d", $precision, $linkindex).'.'.(basename($tuples0[$good_index]->[1]));
	my $link1 = $repository.'/'.sprintf("%0*d", $precision, $linkindex).'.'.(basename($tuples1[$good_index]->[1]));
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
		symlink File::Spec->rel2abs($tuples0[$good_index]->[1]), $link0;
		symlink File::Spec->rel2abs($tuples1[$good_index]->[1]), $link1;
	}
	
	++$linkindex;

}


