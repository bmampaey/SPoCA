#!/usr/bin/perl
use strict;
use threads;
use threads::shared;
use Astro::FITS::CFITSIO;
use Astro::FITS::CFITSIO qw( :longnames );
use Astro::FITS::CFITSIO qw( :constants );


our ($numberWavelength, $resultrepository, $datarepository);
our $spoca;
our %spoca_parameters;
our $tracking;
our %tracking_parameters;
our $getregionstats;
our %getregionstats_parameters;
our $wavelength;
our @files;
our $max_threads = 1;
our $postfix;
our $newColor;
our $getregionstats_outfile;

my($do_classification, $do_tracking, $do_getregionstats, $interactive) = undef;
my($doing_classification, $doing_tracking, $doing_getregionstats) = undef;
sub output_argumentfile
{

my $default_config_file = q^

# This is an example configuration file for using with runspoca.pl
# You can modify it or better copy it and modify your own copy
# It is important to keep the structure because it is perl code and will be interpreted as such 

# The number of wavelength
$numberWavelength = 2;

# The repository for the results (can be passed at the command line)
$resultrepository = 'results';

# The repository containing the data (can be passed at the command line)
$datarepository = '/home/benjamin/data/eit/200305';


### Spoca arguments ###

# The spoca program
$spoca = 'bin/classification.x ';

# The parameters to the spoca program
%spoca_parameters = (

# The type of classifier to use for the classification.
# Possible values are : FCM, PCM, PCM2, SPOCA, SPOCA2, HFCM(Histogram FCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)
classifierType => 'HFCM',
	
# The fuzzifier
fuzzifier => '2',

# The maximal number of iteration for the classification.
maxNumberIteration => '100',
	
# The precision to be reached to stop the classification.
precision => '0.0001',

# The name of the file containing the centers.
centersFile => 'centers.txt',
	
# The number of classes to classify the sun images into. 
numberClasses => '3',
	
	
# The type of the images. 
# Possible values are : EIT, EUVI, AIA, SWAP
imageType => 'EIT',

# The steps of preprocessing to apply to the sun images (comma separated list of string (no spaces))
#Possible values :
#		NAR (Nullify above radius)
#		ALC (Annulus Limb Correction)
#		DivMedian (Division by the median)
#		TakeSqrt (Take the square root)
#		TakeLog (Take the log)
#		DivMode (Division by the mode)
#		DivExpTime (Division by the Exposure Time)
preprocessingSteps => 'ALC',	
	
# The ratio of the radius of the sun that will be processed.
radiusratio => '1.3',
	
# The neighboorhoodRadius is half the size of the square of neighboors (Only for spatial classifiers like SPoCA.)
neighboorhoodRadius => '1',

# The size of the bins of the histogramm.
binSize => '5,5',
	
# The name of a file containing an histogram.
histogramFile => 'histogram.txt',


);

# The tracking program
$tracking = 'bin/tracking.x';

# The postfix of the file to be tracked
$postfix = 'ARmap.tracking.fits';

# The first color to assign to a region
$newColor = 0 ;

%tracking_parameters = (

# The number of images that overlap between 2 tracking run
overlap => 3 ,

# The type of the images (should be the same as for spoca)
imageType => $spoca_parameters{imageType} ,
	

# The maximal delta time between 2 tracked regions
delta_time => 3600 ,

# Whether to color all images or not
writeAllImages => ' ' 


);

# The getregionstats program
$getregionstats = 'bin/get_regions_stats.x';

# The wavelength for the getregionstats output
$wavelength = 171;

# The file to write the getregionstats output
$getregionstats_outfile = "./results/regions_stats.txt";

%getregionstats_parameters = (

# The number of images that overlap between 2 tracking run
coordinateType => 'HGC' ,

# The type of the images (should be the same as for spoca)
imageType => $spoca_parameters{imageType} ,

# The steps of preprocessing to apply to the sun images (comma separated list of string (no spaces))
#Possible values :
#		NAR (Nullify above radius)
#		ALC (Annulus Limb Correction)
#		DivMedian (Division by the median)
#		TakeSqrt (Take the square root)
#		TakeLog (Take the log)
#		DivMode (Division by the mode)
#		DivExpTime (Division by the Exposure Time)
preprocessingSteps => 'NAR',	
	
# The ratio of the radius of the sun that will be processed.
radiusratio => '0.95',

);


^;
print STDOUT $default_config_file;
}

sub parse_arguments
{
	my $help = undef;
	my $help_config = undef;

	my ($arg_resultrepository, $arg_datarepository, $arg_configfile) = undef;
	use Getopt::Long;
	GetOptions(
            "r:s"=>\$arg_resultrepository,
            "d:s"=>\$arg_datarepository,
            "a:s"=>\$arg_configfile,
            "h"=>\$help,
            "f"=>\$help_config,
            "c"=>\$do_classification,
            "t"=>\$do_tracking,
            "g"=>\$do_getregionstats,
            "p:i"=>\$max_threads
	);
	

	if (! defined $arg_configfile && @ARGV > 0)
	{
		$arg_configfile = shift @ARGV;		
	}
	if( ! defined $arg_configfile)
	{
		$help = 1;
	}
	else
	{
		unless(my $return = do $arg_configfile)
		{
			die "couldn't parse $arg_configfile: $@" if $@;
			die "couldn't do $arg_configfile: $!" unless defined $return;
			die "couldn't run $arg_configfile" unless $return;	
		}
	}
	
	if ($arg_resultrepository)
	{
		$resultrepository = $arg_resultrepository;		
	}
	
	
	if ($arg_datarepository)
	{
		$datarepository = $arg_datarepository;		
	}
	if($datarepository)
	{
		@files = `ls $datarepository/*.fits $datarepository/*.fts 2>/dev/null`;
		chomp foreach (@files);
	}
	elsif(@ARGV)
	{
		@files = @ARGV;
	}
	else
	{
		$help = 1;
	}
	
	if (!$help_config && $help)
	{
		print STDERR "Usage: runspoca.pl -a argument_file [-p number_of_threads] [-r resultrepository] {-d datarepository | filename1 filename2 ...} \n";
		print STDERR "This script will run a classification, then tracking, then get_region_stats\n";
		print STDERR "For a canvas of the argument_file call with -f\n";
		print STDERR "To disable the interactive mode specify a combination of:\n\t-c (classification)\n\t-t (tracking)\n\t-g (get_regions_stats)\n";

	}
	if ($help_config)
	{
		output_argumentfile;
	}
	if ($help || $help_config)
	{
		exit(2);
	}
}

parse_arguments();

my $all = 0 ;
my $yes = 0;
my $answer = "no";
$interactive = !($do_classification or $do_tracking or $do_getregionstats);

# We create the file_sets
my @file_sets;

for (my $i = 0; $i < @files; )
{
	my @fileset;
	for (my $w = 0; $w < $numberWavelength; ++$w)
	{
		push @fileset, $files[$i];
		++$i;
		
	}
	push @file_sets, [@fileset];
}


# We create the list of spoca to run

my $parameters;
foreach(keys %spoca_parameters)
{
	if($spoca_parameters{$_})
	{
		$parameters .= " --$_ " . $spoca_parameters{$_};
	}
}



my @tracking_files;
my @classification_threads;


my $numwidth = (log(@file_sets)/log(10)) + 1;
for (my $i = 0; $i < @file_sets; ++$i)
{
	my $num = sprintf("%0*d", $numwidth, $i);

	my $output = "$resultrepository/$num";
	my $fits = (join ' ', @{@file_sets[$i]} );
	my $execute = "$spoca $parameters -O $output $fits";
		
	if ($interactive and ! $all)
	{
		print "\nAbout to do : \n\t$execute\nIs it ok ? [none/no/yes/all] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^all/i) ? 1 : 0;
		last if ($answer =~ /^none/i);
	
	}
	if ((!$interactive and $do_classification) or $all or $yes)
	{ 
			push @classification_threads, $execute;
			push @tracking_files, $output;
		
	}
	
}

$doing_classification = @classification_threads;




# We run the tracking

$all = 0;
$yes = 0;
$answer = "no";

my @tracking_threads;
my $maxfiles = $tracking_parameters{overlap} * 3;
my $lastcolor :shared;
$lastcolor = $newColor ? $newColor : 0;
my $number_files = 0;
if(! @tracking_files)
{
	@tracking_files = `ls -1 $resultrepository/*$postfix`;
	chomp foreach (@tracking_files);
	$number_files = @tracking_files;
}
else
{
	for (my $i = 0; $i < @tracking_files; ++$i)
	{
		$tracking_files[$i]=$tracking_files[$i].'.'.$postfix;
	}
}

my @colormaps=@tracking_files;

$parameters = '';
foreach(keys %tracking_parameters)
{
	if($tracking_parameters{$_})
	{
		$parameters .= " --$_ " . $tracking_parameters{$_};
	}
}


my @reference;
while (@reference < $tracking_parameters{overlap})
{
	push @reference, shift @tracking_files;
}

while ( @tracking_files )
{

	my @tracking_set;
	for (my $i = 1; $i <= $maxfiles - $tracking_parameters{overlap} && @tracking_files; ++$i)
	{
		push @tracking_set, shift @tracking_files;
	}
	
	my $execute = "$tracking $parameters @reference @tracking_set";
	
	if ($interactive and ! $all)
	{
		print "\nAbout to do : \n\t$execute\nIs it ok ? [none/no/yes/all] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^all/i) ? 1 : 0;
		last if ($answer =~ /^none/i);
	
	}
	if ((!$interactive and $do_tracking) or $all or $yes)
	{ 
			push @tracking_threads, $execute;
		
	}
	
	
	@reference = ();
	while (@reference < $tracking_parameters{overlap})
	{
		unshift @reference, pop @tracking_set;
	}
	
}

$doing_tracking = @tracking_threads;


# We do the get_regions_stats

$all = 0;
$yes = 0;
$answer = "no";

$parameters = '';
foreach(keys %getregionstats_parameters)
{
	if($getregionstats_parameters{$_})
	{
		$parameters .= " --$_ " . $getregionstats_parameters{$_};
	}
}


my @getregionstats_threads;
for (my $i = 0; $i < @colormaps && $i < @file_sets; ++$i) 
{
	my $sunfile = undef;
	foreach my $filename( @{@file_sets[$i]} )
	{
		my ($fitsfile, $status) = undef;
		fits_open_image($fitsfile, $filename, READONLY,$status);
		if($status)
		{
			warn "error opening file $filename";
			next;
		}

		my ($hash_ref) = $fitsfile->read_header;
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
		if($wavelnth == $wavelength)
		{
			$sunfile = $filename;
			last;
		}
	}
	if(!$sunfile)
	{
		warn "Error couldn't find a fits file with wavelength $wavelength among  @{@file_sets[$i]}\n Skipping $colormaps[$i].\n";
		next;
	}
	
	my $execute = "$getregionstats $parameters -M $colormaps[$i] $sunfile";
	
	if ($interactive and ! $all)
	{
		print "\nAbout to do : \n\t$execute\nIs it ok ? [none/no/yes/all] :";
		$answer = <STDIN>;
		chomp $answer;
		$yes =  ($answer =~ /^y/i) ? 1 : 0;
		$all = 	($answer =~ /^all/i) ? 1 : 0;
		last if ($answer =~ /^none/i);
	
	}
	if ((!$interactive and $do_getregionstats) or $all or $yes)
	{ 
			push @getregionstats_threads, $execute;
		
	}
	
	
}

$doing_getregionstats = @getregionstats_threads;

sub do_execute
{
	my $execute = join ' ', @_;
	print "START $execute\n";
	my $output = `$execute`;
	if ($? >> 8 != 0)
	{
		die "There was an error doing $execute : $output\n";
	}
	print "END $execute\n";
	#system("sync");
	return $output;
} 
sub do_tracking
{
	lock($lastcolor);
	my $output = do_execute @_ , "-n $lastcolor";
	if($output =~ m#(\d+)#)
	{
		$lastcolor = $1;
	}
	else
	{
		die "The last color was not present in the output : $output"
	}
	return $output;
} 



my @job_list;

while (@classification_threads)
{
		push @job_list, {SUB => 'do_execute', ARGS => shift @classification_threads, TYPE => "classification"};
}
if(!$doing_classification)
{
	while (@tracking_threads)
	{
			push @job_list, {SUB => 'do_tracking', ARGS => shift @tracking_threads, TYPE => "tracking"};
	}
}
if(!$doing_classification && !$doing_tracking)
{
	while (@getregionstats_threads)
	{
			push @job_list, {SUB => 'do_execute', ARGS => shift @getregionstats_threads, TYPE => "getregionstats"};
	}
}

my @thread_list;
if($doing_getregionstats)
{
	open FILE, ">", $getregionstats_outfile;
}

while (@job_list || @thread_list)
{
	while(@job_list && @thread_list < $max_threads)
	{
		my $job = shift @job_list;
		my $thr = threads->create( $job->{SUB}, $job->{ARGS});
		push @thread_list, {THREAD => $thr, TYPE => $job->{TYPE}};
	}
	my $thread = shift @thread_list;
	my $output = $thread->{THREAD}->join();
	if($thread->{TYPE} eq "classification")
	{
		++$number_files;
		if(@tracking_threads && $number_files >= $maxfiles)
		{
			$number_files -= $maxfiles - $tracking_parameters{overlap};
			unshift @job_list, {SUB => 'do_tracking', ARGS => shift @tracking_threads, TYPE => "tracking"};
		}
	}
	if($thread->{TYPE} eq "tracking")
	{
		if(@getregionstats_threads)
		{
			for(my $i = 0; $i < $maxfiles - $tracking_parameters{overlap} && @getregionstats_threads; ++$i)
			{
				
				unshift @job_list, {SUB => 'do_execute', ARGS => shift @getregionstats_threads, TYPE => "getregionstats"};
			}
		}
	}
	if($thread->{TYPE} eq "getregionstats")
	{
		print FILE $output, "\n";
	}
}

while (@tracking_threads)
{
	my $thr = threads->create('do_tracking', shift @tracking_threads);
	$thr->join();
}

while(@getregionstats_threads)
{
	push @job_list, {SUB => 'do_execute', ARGS => shift @getregionstats_threads, TYPE => "getregionstats"};
}

while (@job_list || @thread_list)
{
	while(@job_list && @thread_list < $max_threads)
	{
		my $job = shift @job_list;
		my $thr = threads->create( $job->{SUB}, $job->{ARGS});
		push @thread_list, {THREAD => $thr, TYPE => $job->{TYPE}};
	}
	my $thread = shift @thread_list;
	my $output = $thread->{THREAD}->join();
	if($thread->{TYPE} eq "getregionstats")
	{
		print FILE $output, "\n";
	}
}

close FILE;



