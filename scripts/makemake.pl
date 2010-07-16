#!/usr/bin/perl

use strict;
use File::Basename;

# My global variable

my ($main, $makefile, $debug, $defines, $executable) = undef;
my $objectdir = 'objects';
my $bindir = 'bin';

my $CC = 'CC=g++';
my $CFLAGS = 'CFLAGS=-Wall -fkeep-inline-functions';
my $LFLAGS = 'LFLAGS=-lcfitsio';
my $DFLAGS = 'DFLAGS=';
my $TRACKINGFLAGS = 'TRACKINGFLAGS=-lpthread';
my $IDLFLAGS = 'IDLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a';


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
	GetOptions(
            "m:s"=>\$main,
            "o:s"=>\$makefile,
            "d"=>\$debug,
            "D:s"=> \$defines,
            "x:s"=> \$executable
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
	print $debug . "\n";
	if ($debug)
	{
		$CFLAGS .= ' -g';
	}
	else
	{
		$CFLAGS .= ' -O3';
	}
	if (! defined $executable)
	{
		$executable = fileparse($main, @suffixes);
		$executable = $bindir . '/' . $executable . '.x';
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
	my @includes;
	open FILE, "<$file" or die "Could not open $file" ;
	while (<FILE>)
	{ 
		if (m/#include\s+"(.*)"/)
		{
			push @includes, $1;
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
		
		my $line = "$object : $makefile $cpp " . (join ' ', @includes) . "\n";
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

print MAKEFILE "$CC\n$CFLAGS\n$LFLAGS\n$TRACKINGFLAGS\n$IDLFLAGS\n$DFLAGS\n\n";
print MAKEFILE "all:$executable\n";
print MAKEFILE "clean: rm $executable " . (join ' ', @objects) . "\n";
print MAKEFILE "\n\n";
print MAKEFILE "$executable : $makefile " . (join ' ', @objects) . "\n";
print MAKEFILE "\t" . '$(CC) $(CFLAGS) $(DFLAGS) ' . (join ' ', @objects) . ' $(LFLAGS) -o ' . $executable . "\n\n";
print MAKEFILE (join "\n", @lines);

close MAKEFILE;

