#!/usr/bin/perl
use strict;

my $processing = ' -alpha set -fill none -draw "color 0,0 replace" ';
my $annotation = ' -fill white -pointsize 20 -gravity northeast -undercolor black -font helvetica -annotate 0 ';
my $convert = 'convert ';
my $composite = 'composite ';
my $gradient = "scripts/gradient.png";

my $movierepositoryprefix = 'movie';
my $resultsrepositoryprefix = 'results';
my $backgroundrepositoryprefix = 'background';
my $datarepository = '/home/benjamin/data/eit/200305';
my @wavelengths = ("171", "195");
my @algos = ("");
my $fitssuffix = 'fits';

my $all = 0;
my $yes = 0;
my $answer = "no";



	my $resultsrepository = $resultsrepositoryprefix;
	# We draw the contours	

	my @files = `ls -1 $resultsrepository/*.tracking.tracked.fits`;
	chomp foreach(@files);

	$all = 0;
	$yes = 0;
	print "\nAbout to do : \n../spoca_ben/bin/ImageContour.x @files\nIs it ok ? [yes/no/all/none] :";
	$answer = <STDIN>;
	chomp $answer;
	$yes =  ($answer =~ /^y/i) ? 1 : 0;
	$all = 	($answer =~ /^a/i) ? 1 : 0;
	if ($all or $yes)
	{ 
		print `../spoca_ben/bin/ImageContour.x @files 2>&1`."\n\n";
	}




	# We make the images readable by ImageMagick
	@files = `ls -1 $resultsrepository/*.tracking.tracked.contours.fits`;
	chomp foreach(@files);

	$all = 0;
	$yes = 0;
	print "\nAbout to do : \n../spoca_ben/bin/ImageMagick.x @files\nIs it ok ? [yes/no/all/none] :";
	$answer = <STDIN>;
	chomp $answer;
	$yes =  ($answer =~ /^y/i) ? 1 : 0;
	$all = 	($answer =~ /^a/i) ? 1 : 0;
	if ($all or $yes)
	{ 
		print `../spoca_ben/bin/ImageMagick.x @files 2>&1`."\n\n";
	}



	# We colorize the images and make the background transparent

	$all = 0;
	$yes = 0;

	@files = `ls -1 $resultsrepository/*.tracking.tracked.contours.magick.fits`;
	chomp foreach(@files);
	for(my $i = 0; $i < @files; ++$i)
	{	
		$files[$i] =~ m#([^/]*)\.fits$#;
		my $name = $resultsrepository .'/'. $1 . '.colorized.png';
		
		my $colorize = $convert . $files[$i] . " " . $gradient . ' -clut ' . $name;
		my $hollow = $convert . $name . $processing . $name;
		my $annotate = "";
		if (! $all)
		{
			print "\nAbout to do : \n\t$colorize\n\t$hollow\n\t$annotate\nIs it ok ? [yes/no/all/none] :";
			$answer = <STDIN>;
			chomp $answer;
			$yes =  ($answer =~ /^y/i) ? 1 : 0;
			$all = 	($answer =~ /^a/i) ? 1 : 0;
			last if ($answer =~ /none/i);
		
		}
		if ($all or $yes)
		{ 
			print "$colorize\n" . `$colorize 2>&1`."\n";
			print "$hollow\n" . `$hollow 2>&1`."\n";
			print "$annotate\n" . `$annotate 2>&1`."\n";
		}
	 	
	}

	#Now that everithing is ready we can compose the contours on top of the background

	for my $wavelength (@wavelengths)
	{
		my $backgroundrepository = $backgroundrepositoryprefix.'.'.$wavelength;
		my $movierepository = $movierepositoryprefix.'.'.$wavelength;
		mkdir $movierepository;


		$all = 0;
		$yes = 0;

		@files = `ls -1 $backgroundrepository/*.background.png`;
		chomp foreach(@files);

		for(my $i = 0; $i < @files; ++$i)
		{	
			$files[$i] =~ m#(\d+)\.#;
			my $name = $movierepository . '/' . $1 . '.frame.png';
			my @contours = `ls -1 $resultsrepository/$1*tracking.tracked.contours.magick.colorized.png`;
			
			die "Found several contours for backgroung image ".$files[$i]." :\n@contours\n" if (@contours > 1);
			if (@contours == 0 )
			{
				my $annotate = $convert . $files[$i] . $annotation . '"No results" '.$name;
				print "Not found contours file, doing\n\t$annotate\n" . `$annotate 2>&1`."\n";
				next;
			}
			my $contour = $contours[0];
			chomp $contour;
			
			my $assemble = $composite . ' -gravity center ' . $contour . ' ' . $files[$i] . ' ' . $name;
			
			if (! $all)
			{
				print "\nAbout to do : \n\t$assemble\nIs it ok ? [yes/no/all/none] :";
				$answer = <STDIN>;
				chomp $answer;
				$yes =  ($answer =~ /^y/i) ? 1 : 0;
				$all = 	($answer =~ /^a/i) ? 1 : 0;
				last if ($answer =~ /none/i);
			
			}
			if ($all or $yes)
			{ 
				print "$assemble :\n" . `$assemble 2>&1`."\n";
			}
		}

		print "Review the frames then to create the movie type :\n";
		print "convert -delay 50 -loop 0 $movierepository/*.frame.png $movierepository/movie.gif\n";
		print "convert -delay 100 $movierepository/movie.gif $movierepository/slow.movie.gif\n";
		print "convert -resize 50% $movierepository/movie.gif $movierepository/small.movie.gif\n";
	}

	 

