#!/usr/bin/perl
#
# File: rename.pl
# Written by Benjamin Mampaey on 30 April 2009
# This script will rename an AIA fits file


use strict;
use File::Basename;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :constants );
use DateTime;



my $interactive = 0;
my $prefix = 'AIA';
my @filenames;
my $directory = undef;
my $number_process = 1;

my $all = 0;
my $yes = 0;
my $answer = "no";

sub parse_arguments
{
	my $help = undef;
	my $batch = undef;
	if (@ARGV == 0)
	{
		$help = 1;
	}
	else
	{
		use Getopt::Long;
		GetOptions(
	            "p:s"=>\$prefix,
	            "l:s"=>\$directory,
	            "h"=>\$help,
	            "b"=>\$interactive,
	            "c:i"=>\$number_process,
		);
	}

	if (! defined $prefix)
	{
		$help = 1;		
	}
	
	$all = $batch;
	$interactive = !$batch;
	
	if(! defined $number_process)
	{
		$number_process = 1;		
	}
	elsif($number_process > 1)
	{
		$all = 1;
	}
	
	
	if( defined $directory && ! -d $directory && ! -w $directory)
	{
		print STDERR "$directory does not exists or is not writable";
	}

	@filenames = @ARGV or $help = 1;
	
	if ($help == 1)
	{
		print STDERR "Usage: rename.pl [-b] [-l directory] [-p prefix] [-c number_process] filenames_to_be_renamed\n";
		print STDERR "This script renames fits files (make links to them) with names like so: prefix.yyyymmdd_hhmmss.wavelength.fits\n";
		print STDERR "The prefix can be set up using the -p option, by default it is \"$prefix\"\n";
		print STDERR "If you want links instead of renaming the files, provide the directory\n";
		print STDERR "Specify -b if you don't want to be prompted\n";
		print STDERR "If you want to run it concurrently, specify the number of process with the option -c (only in batch mode)\n";
		exit(2);
	
	}
}



sub get_keywords
{
	my $filename = shift;
	my @keywords = @_;
	
	my $status = 0;
	my $header;
	{
		my $fitsfile;
    	fits_open_image($fitsfile, $filename, READONLY, $status);
    	if($status)
    	{
    		warn "error opening file $filename" if $interactive;
    		return undef;
    	}

    	($header, $status) = $fitsfile->read_header;
    	if($status)
    	{
    		warn "error reading header from file $filename" if $interactive;
    		return undef;
    	}
    }
	my @values;
	for my $keyword (@keywords)
	{
		my $value = $header->{$keyword};
		if (! $value)
		{
			warn "Missing keyword $keyword in file $filename" if $interactive;
			$value = undef;
		}
		push @values, $value;
	}
	return @values;
}

sub get_date
{
	my $date = shift;
	unless ($date =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)#)
	{
		warn "$date is not interpretable" if $interactive;
		return undef; 
	}
	return DateTime->new(
      year       => $1,
      month      => $2,
      day        => $3,
      hour       => $4,
      minute     => $5,
      second     => $6,
      nanosecond => 0,
      time_zone  => 'UTC',
  );

}

sub make_name
{
	my $prefix = shift;
	my $date = shift;
	my $wavelength = shift;
	my $suffix = shift;
	my $date_string = sprintf("%04d", $date->year).sprintf("%02d", $date->month).sprintf("%02d", $date->day).'_'.sprintf("%02d", $date->hour).sprintf("%02d", $date->minute).sprintf("%02d", $date->second);
	my $wavelength_string = sprintf("%04d", $wavelength);
	return join ('.', $prefix,$date_string,$wavelength_string,$suffix);
}

sub make_path
{
	my $directory = shift;
	my $date = shift;
	my $subdirectory = $directory;
	$subdirectory .= '/'.sprintf("%04d", $date->year);
	mkdir $subdirectory if (! -d $subdirectory);
	$subdirectory .= '/'.sprintf("%02d", $date->month);
	mkdir $subdirectory if (! -d $subdirectory);
	$subdirectory .= '/'.sprintf("%02d", $date->day);
	mkdir $subdirectory if (! -d $subdirectory);
	$subdirectory .= '/H'.sprintf("%02d", $date->hour).'00';
	mkdir $subdirectory if (! -d $subdirectory);
	
	return $subdirectory;
}

sub make_link
{
	my $filename = shift;
	my $directory = shift;
	my $prefix = shift;
	my ($date_obs, $fits_date, $wavelength) = get_keywords($filename, 'T_OBS', 'DATE', 'WAVELNTH') or return undef;

	my $date = get_date($date_obs) or return undef;

	my $newfilename = make_path($directory, $date) . '/' . make_name($prefix,$date,$wavelength,'fits');
	
	if (! $all)
	{
		print "\nAbout to link $filename to $newfilename\nIs it ok ? [yes/no/all/none] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^a/i) ? 1 : 0;
		last if ($answer =~ /none/i);
	
	}
	if ($all or $yes)
	{ 
		if (-l $newfilename)
		{
			my ($oldfits_date) = get_keywords($newfilename, 'DATE');
			my $old_date = get_date($oldfits_date);
			unless (defined $old_date)
			{
				$old_date = DateTime->new(year => 0);
			}
			my $new_date = get_date($fits_date);
			if(DateTime->compare($old_date, $new_date) < 0)
			{
				warn "$newfilename already exists, but is older than $filename. Replacing it.\n" if $interactive;
				if (unlink $newfilename)
				{
					if (! symlink $filename, $newfilename) 
					{
						warn "Couldn't make link $newfilename -> $filename : $!" if $interactive;
					}
				}
				else
				{
					warn "Couldn't suppress link $newfilename : $!" if $interactive;
				}
			}
			else
			{
				warn "$newfilename already exists, but is not older than $filename. Not replacing it.\n" if $interactive;
			}
		}
		elsif(! -e $newfilename)
		{
			if (! symlink $filename, $newfilename) 
			{
				warn "Couldn't make link $newfilename -> $filename : $!" if $interactive;
			}
		}
		else
		{
			warn "$newfilename already exists and is not a symbolic link. Skipping\n" if $interactive;
		}
		
	}
}

sub rename_file
{
	my $filename = shift;
	my $prefix = shift;
	my ($date_obs, $fits_date, $wavelength) = get_keywords($filename, 'T_OBS', 'DATE', 'WAVELNTH') or return undef;

	my $date = get_date($date_obs) or return undef;
	my ($junk, $directory) = fileparse($filename);
	my $newfilename = $directory . '/' . make_name($prefix,$date,$wavelength,'fits');
	
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

    	if (! rename($filename, $newfilename)) 
    	{
    		warn "Couldn't rename file $filename : $!" if $interactive;
    	}
	}
}

parse_arguments();

# Because ASTRO::Fits is not good with threads, we need to fork
my $max_files = int(@filenames / $number_process);
my $lower_limit = 0;
my $upper_limit = $max_files;

if(@filenames > $number_process)
{
    while($number_process > 1)
    {
    	my $pid = fork;
    	unless (defined $pid)
    	{
    		warn "Couldn't fork process: $!";
    		$upper_limit += $max_files;
    		--$number_process;
    	}
    	if($pid) # I am the parent
    	{
    		$lower_limit = $upper_limit + 1;
    		$upper_limit += $max_files;
    		--$number_process;
    	}
    	else
    	{
    		$number_process = 0;
    	}
    }
}

if($number_process >= 1)
{
	$upper_limit = $#filenames;
}

foreach my $filename (@filenames[$lower_limit..$upper_limit])
{
	if(defined $directory)
	{
		make_link($filename, $directory, $prefix) ;
	}
	else
	{
		rename_file($filename, $prefix);
	}
}

if($number_process >= 1)
{
	while(wait() != -1){}
}


