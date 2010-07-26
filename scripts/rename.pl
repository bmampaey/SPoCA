#!/usr/bin/perl
#
# File: rename.pl
# Written by Benjamin Mampaey on 30 April 2009
# This script will rename an AIA fits file


use strict;
use File::Basename;
use Data::Dumper;

use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :constants );

my $all = 0;
my $yes = 0;
my $answer = "no";

my $prefix = 'aia';
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
	            "p:s"=>\$prefix,
	            "h"=>\$help,
		);
	}

	if (! defined $prefix)
	{
		$help = 1;		
	}

	@filenames = @ARGV or $help = 1;
	
	if ($help == 1)
	{
		print STDERR "Usage: rename.pl [-p prefix] filenames_to_be_renamed\n";
		print STDERR "This script renames fits files like so: prefix.yyyymmdd_hhmmss.wavelength.fits\n";
		print STDERR "The prefix can be set up using the -p option, by default it is \"$prefix\"\n";
		exit(2);
	
	}
}

parse_arguments();

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
	$date_obs =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)#;
	my $date = "$1$2$3_$4$5$6";
	my $wavelength = sprintf("%0*d", 4, $wavelnth);
	my ($junk, $directory) = fileparse($filename);
	my $newfilename = $directory.$prefix.'.'.$date.'.'.$wavelength.'.fits';
	if (! $all)
	{
		print "\nAbout to rename $filename to $newfilename\nIs it ok ? [yes/no/all/none] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^a/i) ? 1 : 0;
		last if ($answer =~ /none/i);
	
	}
	if ($all or $yes)
	{ 
		rename $filename, $newfilename or warn "Couldn't rename file $filename : $!";
	}

	#print Dumper($hash_ref);
}


