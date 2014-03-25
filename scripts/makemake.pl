#!/usr/bin/perl

use strict;
use File::Basename;

# My global variable

my ($main, $makefile, $debug, $defines, $executable, $coco, $magick) = undef;
my $objectdir = 'objects';
my $bindir = 'bin';

my $CC = 'CC=g++';
my $CFLAGS = 'CFLAGS=-Wall -fkeep-inline-functions -g';
my $LFLAGS = 'LFLAGS=-lcfitsio';
my $DFLAGS = 'DFLAGS=';
my $TRACKINGLFLAGS = 'TRACKINGLFLAGS=-lpthread';
my $MAGICKLFLAGS = 'MAGICKLFLAGS=`Magick++-config --ldflags --libs`';
my $MAGICKCFLAGS = 'MAGICKCFLAGS=`Magick++-config --cppflags`';
my $IDLLFLAGS = 'IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a';


my @suffixes = ('.c', '.cpp', '.h', '.x', '.o', '.hpp');


sub strip
{
	my $file = shift;
	my($filename, $directories, $suffix) = fileparse($file);
	$file = $directories . shift;
	
	my @path = split '/', $file;
	for (my $i = 0; $i < @path; ++$i)
	{
		if( $path[$i] eq '.')
		{
			$path[$i] = undef;
		}
		elsif( $path[$i] eq '..')
		{
			my $j = $i - 1;
			 --$j while ( !$path[$j] and $j >= 0);
			$path[$j] = undef;
			$path[$i] = undef;
		}	
	}
	shift @path while (! $path[0]);
	
	my $result =  shift @path;
	
	foreach (@path)
	{
		$result .= '/' . $_ if ($_);
	}
	return $result;
}

sub parse_arguments
{
	use Getopt::Long;
	Getopt::Long::Configure("no_ignore_case");
	GetOptions(
            "m:s"=>\$main,
            "o:s"=>\$makefile,
            "d"=>\$debug,
            "D:s"=> \$defines,
            "x:s"=> \$executable,
            "C"=> \$coco,
            "M"=> \$magick,
	);

	if (! defined $main)
	{
		if (@ARGV > 0)
		{
			$main = $ARGV[0];
		}
		else
		{
			die "You must provide the file containing the main with -m";
		}
	}
	die "$main does not exist!" if( ! -f $main);
	
	if (! defined $makefile)
	{
		$makefile = fileparse($main, @suffixes);
		$makefile .= '.mk';
	}
	
	$main = strip $makefile, $main;
	
	if (defined $defines)
	{
		$DFLAGS .= $defines;
	}
	if($debug)
	{
		$DFLAGS .= " -DDEBUG";
		print "Debug turned on\n";
	}
	
	if (! $debug)
	{
		$CFLAGS .= ' -O3';
	}
	if (! defined $executable)
	{
		$executable = fileparse($main, @suffixes);
		$executable = $bindir . '/' . $executable . '.x';
	}
	if($coco)
	{
		$DFLAGS = $DFLAGS . ' -DCOCO ';
	}
	if($magick)
	{
		$CFLAGS = $CFLAGS . ' $(MAGICKCFLAGS) ';
		$LFLAGS = $LFLAGS . ' $(MAGICKLFLAGS) ';
		$DFLAGS = $DFLAGS . ' -DMAGICK ';
	}

}




sub object
{
	my $file = shift;
	my($filename, $directories, $suffix) = fileparse($file, @suffixes);
	if( -f $directories.$filename.'.cpp' || -f $directories.$filename.'.c')
	{
		return $objectdir.'/'.$filename.'.o';
	}
	else
	{
		return undef;
	}
	
}
sub cpp
{
	my $file = shift;
	my($filename, $directories, $suffix) = fileparse($file, @suffixes);
	if( -f $directories.$filename.'.cpp')
	{
		return $directories.$filename.'.cpp';
	}
	elsif (-f $directories.$filename.'.c')
	{
		return $directories.$filename.'.c';
	}
	else
	{
		return undef;
	}
	
}



parse_arguments();

my @treated;
my @untreated = $main;
my @lines;
my @objects;

while (my $file = shift @untreated)
{

	if($file =~ /tracking/i)
	{
		$LFLAGS .= '  $(TRACKINGLFLAGS)';
	}
	if($file =~ /CoordinateConvertor/i)
	{
		$LFLAGS .= '  $(IDLLFLAGS)';
	}

	my @includes;
	open FILE, "<$file" or die "Could not open $file" ;
	while (<FILE>)
	{ 
		if (m/^#include\s+"(.*)"/)
		{
			my $include = $1;
			my $skip = undef;
			if($include =~ /CoordinateConvertor/i && !$coco)
			{
				$skip = 1;
			}
			if ($include =~ /MagickImage/i && !$magick)
			{
				$skip = 1;
			}
			if($skip)
			{
				print "Skipping $include.\n"
			}
			else
			{
				push @includes, $include;
			}
		}
	} 
	close FILE;
	
	for(my $i = 0; $i < @includes; ++$i)
	{
		$includes[$i] = strip ($file, $includes[$i]);
		die "No file $includes[$i] included from $file" if(! -f $includes[$i]);
	}
	
	my $cpp = cpp $file;
	
	if( defined $cpp)
	{
		my $object = object $cpp;
		
		my $line = "$object : $makefile $cpp " . (join ' ', @includes) . "| $objectdir\n";
		$line .= "\t" . '$(CC) -c $(CFLAGS) $(DFLAGS) ' . "$cpp -o $object\n"; 
		
		push @lines, $line;
		push @objects, $object;
		
	}
	
	push @treated, $file;
	
	INCLUDE : foreach my $include (@includes)
	{	
		foreach (@treated, @untreated)
		{
			next INCLUDE if ($_ eq $include);
		}
		
		unshift @untreated, $include;
	}
	
	
}

open MAKEFILE, ">$makefile" or die "Could not open $makefile";

print MAKEFILE "$CC\n$TRACKINGLFLAGS\n$IDLLFLAGS\n$MAGICKLFLAGS\n$MAGICKCFLAGS\n$CFLAGS\n$LFLAGS\n$DFLAGS\n\n";
print MAKEFILE "all:$executable\n";
print MAKEFILE "clean: rm $executable " . (join ' ', @objects) . "\n";
print MAKEFILE "\n\n";
print MAKEFILE "$executable : $makefile " . (join ' ', @objects) . " | $bindir\n";
print MAKEFILE "\t" . '$(CC) $(CFLAGS) $(DFLAGS) ' . (join ' ', @objects) . ' $(LFLAGS) -o ' . $executable . "\n\n";
print MAKEFILE (join "\n", @lines);
print MAKEFILE "\n$objectdir : \n\t mkdir -p $objectdir\n";
print MAKEFILE "\n$bindir : \n\t mkdir -p $bindir\n";
close MAKEFILE;

