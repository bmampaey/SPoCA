#!/usr/bin/perl
use strict;
use threads;
use threads::shared;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :constants );

my $processing = ' -contrast-stretch 0%x0.1% ';
my $convert = 'convert';
my $uncompress = 'bin/uncompres.x';

my $all = 0;
my $yes = 0;
my $answer = "no";

my $repository = 'background';
my $wavelengthlist;
my @filenames;
my $max_threads = 1;
my $resize;
my $pointsize;

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
	            "p:i"=>\$max_threads,
	            "s:s"=>\$resize,
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
		print STDERR "Usage: makebackgrounds.pl -w wavelenght1,wavelengt2 [-s size] [-p number_of_threads] [-r repository_path] image1.fits image2.fits ... \n";
		print STDERR "This script creates background png pictures.\n";
		print STDERR "The repository where the pictures be created can be specified using the -r option, by default it is the current working directory\n";
		print STDERR "The number of threads can be specified with the -p options\n";
		print STDERR "The size of the pictures can be specified with the -s option. For example -s \"2048x2048\"\n";
		exit(2);

	}
}

parse_arguments();

my @wavelengths = split /,/,$wavelengthlist;


for my $wavelength (@wavelengths)
{
	my $repo = $repository.'.'.$wavelength;
	if(-d $repo && scalar <$repo/*>)
	{
		print "$repo already exists and is not empty. Would you like to continue ? [y/n] ";
		$answer = <STDIN>;
		chomp $answer;
		exit if ($answer !~ /^y/i);
	}
	elsif(! -e $repo)
	{
		mkdir $repo  or die "Error: Could not create directory $repo";
	}
}

if($resize)
{
	if($resize !~ /^(\d+)x(\d+)$/)
	{
		die "Error: $resize is an incorrect size specification. The size must be expressed as widthxheight (ex 2048x2048)."; 
	}
	else
	{
		$pointsize = int($2/40);
	}
}

my @filelist;

foreach my $filename (@filenames)
{

	chomp $filename;
	if (! $all) # We ask the user if he agrees with what we do
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
		my $status2;
		fits_close_file($fitsfile,$status2);
		if($status)
		{
			warn "error reading header from file $filename";
			next;
		}
		
		my $wavelnth = $hash_ref->{'WAVELNTH'};
		if (! $wavelnth )
		{
			warn "Missing keyword WAVELNTH in file $filename";
		   	next;
		   	
		}
		
		# We test is it is a required wavelength
		grep /$wavelnth/, @wavelengths or next;	
		
		my $date_obs;
		# For AIA we prefer the T_OBS
		my $instrume = $hash_ref->{'INSTRUME'};
		if(! $instrume || $instrume !~ /AIA/i)
		{
			$date_obs = $hash_ref->{'DATE-OBS'};
			if (! $date_obs)
			{
				warn "Missing keyword DATE-OBS in file $filename";
				next;
		
			}
		}
		else
		{
			$date_obs = $hash_ref->{'T_OBS'};
			if (! $date_obs)
			{
				warn "Missing keyword T_OBS in file $filename";
				next;
			}
		}
		
		if(! $pointsize)
		{
			my $height = $hash_ref->{'NAXIS2'};
			$pointsize = int($height/40);
		}
		if( $hash_ref->{'ZCMPTYPE'} )
		{
			print "Warning: File $filename seems to be compressed. Will use $uncompress to uncompress it.\n";
			die "$uncompress is not a executable" if(! -x $uncompress);
			
		}

		
		$date_obs =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)# or die "DATE-OBS of file $filename is incorrect : $date_obs";
		my $date_format = "$1$2$3_$4$5$6";
		my $name = "$repository.$wavelnth/$date_format.background.png";
		my $string = "$wavelnth $date_format";
		my $preprocess;
		if ($resize)
		{
			$preprocess = "$convert $filename $processing -resize '$resize' -fill white -pointsize $pointsize -gravity northwest -undercolor black -annotate 0 '$string' -depth 8 $name";
		}
		else
		{
			$preprocess = "$convert $filename $processing -fill white -pointsize $pointsize -gravity northwest -undercolor black -annotate 0 '$string' -depth 8 $name";
		}
		print "\nAbout to do : \n\t$preprocess\nIs it ok ? [yes/no/all/none] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^a/i) ? 1 : 0;
		last if ($answer =~ /none/i);
	
	}
	if ($all or $yes)
	{ 
		push  @filelist, $filename;
	}
	
}

sub do_execute
{

	my $filename = shift;
	my $status;
	my $fitsfile;
	fits_open_image($fitsfile, $filename, READONLY,$status);
	if($status)
	{
		warn "error opening file $filename";
		return undef;
	}

	my ($hash_ref, $status) = $fitsfile->read_header;
	my $status2;
	fits_close_file($fitsfile,$status2);
	if($status)
	{
		warn "error reading header from file $filename";
		return undef;
	}
	
	my $wavelnth = $hash_ref->{'WAVELNTH'};
	if (! $wavelnth )
	{
		warn "Missing keyword WAVELNTH in file $filename";
	   	return undef;
	   	
	}
	
	# We test if it is a required wavelength
	grep /$wavelnth/, @wavelengths or return "";	
	
	my $date_obs;
	# For AIA we prefer the T_OBS
	my $instrume = $hash_ref->{'INSTRUME'};
	if(! $instrume || $instrume !~ /AIA/i)
	{
		$date_obs = $hash_ref->{'DATE-OBS'};
		if (! $date_obs)
		{
			warn "Missing keyword DATE-OBS in file $filename";
			return undef;
	
		}
	}
	else
	{
		$date_obs = $hash_ref->{'T_OBS'};
		if (! $date_obs)
		{
			warn "Missing keyword T_OBS in file $filename";
			return undef;
		}
	}
	
	if(! $pointsize)
	{
		my $height = $hash_ref->{'NAXIS2'};
		$pointsize = int($height/40);
	}

	$date_obs =~ m#(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)# or die "DATE-OBS of file $filename is incorrect : $date_obs";
	my $date_format = "$1$2$3_$4$5$6";
	
	my $compressed = $hash_ref->{'ZCMPTYPE'};
	if($compressed)
	{
		my $filename2 = "$repository.$wavelnth/$date_format.fits";
		my $decompress = "$uncompress $filename $filename2";
		print "$decompress\n";
		my $output = `$decompress`;
		if ($? >> 8 != 0)
		{
			warn "There was an error doing $decompress : $output\n";
			return undef;
		}
		$filename = $filename2;
	}
	
	my $name = "$repository.$wavelnth/$date_format.background.png";
	my $string = "$wavelnth $date_format";
	my $preprocess;
	if ($resize)
	{
		$preprocess = "$convert $filename $processing -resize '$resize' -fill white -pointsize $pointsize -gravity northwest -undercolor black -annotate 0 '$string' -depth 8 $name";
	}
	else
	{
		$preprocess = "$convert $filename $processing -fill white -pointsize $pointsize -gravity northwest -undercolor black -annotate 0 '$string' -depth 8 $name";
	}
	
	print "$preprocess\n";
	my $output = `$preprocess`;
	if ($? >> 8 != 0)
	{
		warn "There was an error doing $preprocess : $output\n";
		return undef;
	}
	
	if($compressed)
	{
		unlink $filename;
	}

	return $output;
} 





my @thread_list;
while (@filelist || @thread_list)
{
	while(@filelist && @thread_list < $max_threads)
	{
		my @args = (shift @filelist);
		my $thr = threads->create( \&do_execute, @args);
		push @thread_list, $thr;
	}
	my $thread = shift @thread_list;
	my $output = $thread->join();
}

