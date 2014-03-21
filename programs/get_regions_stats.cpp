//! This Program computes regions info and statistics.
/*!
@page get_regions_stats get_regions_stats.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/get_regions_stats.x [-option optionvalue ...]  mapFile fitsFile [ fitsFile ... ] </tt>

@param mapFile	Path to the map of regions (i.e. each one must have a different color).
@param fitsFile	Path to a fits file for computing stats

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param append	Set this flag if you want append a new table to the map with the region stats.

@param areaLimit	The type of the limit of the area of the map taken into account the computation of stats. Possible values:
<BR>NAR=zz.z (pixels below zz.z*radius)
<BR>Long=zz.z (pixels between -zz.z and zz.z degrees of longitude)	Lat=zz.z (pixels between -zz.z and zz.z degrees of latitude)

@param colors	The list of color of the regions to plot separated by commas. All regions will be selected if ommited.

@param getRegionsInfo	Set if you want also to get region information.

@param output	The name for the output file or of a directory.

@param registerImages	Set to register/align the images to the map.

@param separator	The separator to put between columns.

@param statsPreprocessing	The steps of preprocessing to apply to the sun images.
<BR>Can be any combination of the following:
<BR> NAR=zz.z (Nullify pixels above zz.z*radius)
<BR> ALC (Annulus Limb Correction)
<BR> DivMedian (Division by the median)
<BR> TakeSqrt (Take the square root)
<BR> TakeLog (Take the log)
<BR> DivMode (Division by the mode)
<BR> DivExpTime (Division by the Exposure Time)
<BR> ThrMin=zz.z (Threshold intensities to minimum zz.z)
<BR> ThrMax=zz.z (Threshold intensities to maximum zz.z)
<BR> ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)
<BR> ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param totalStats	Set this flag if you want to get stats on all regions taken together. This will actually compute segmentation stats.
See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <set>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"

#include "../classes/SegmentationStats.h"
#include "../classes/RegionStats.h"

using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program computes regions info and statistics.";
	programDescription+="\nVersion: 3.0";
	programDescription+="\nAuthor: Benjamin Mampaey, benjamin.mampaey@sidc.be";
	
	programDescription+="\nCompiled on "  __DATE__  " with options :";
	programDescription+="\nNUMBERCHANNELS: " + toString(NUMBERCHANNELS);
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\nVERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());
	
	// We define our program parameters
	ArgParser args(programDescription);
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["getRegionsInfo"] = ArgParser::Parameter(false, 'r', "Set if you want also to get region information.");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=0.95", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images to the map.");
	args["areaLimit"] = ArgParser::Parameter("none", 'L', "The type of the limit of the area of the map taken into account the computation of stats. Possible values:\nNAR=zz.z (pixels below zz.z*radius)\nLong=zz.z (pixels between -zz.z and zz.z degrees of longitude)\tLat=zz.z (pixels between -zz.z and zz.z degrees of latitude)");
	args["append"] = ArgParser::Parameter(false, 'a', "Set this flag if you want append a new table to the map with the region stats.");
	args["totalStats"] = ArgParser::Parameter(false, 't', "Set this flag if you want to get stats on all regions taken together. This will actually compute segmentation stats.");
	args["separator"] = ArgParser::Parameter(',', 's', "The separator to put between columns.");
	args["colors"] = ArgParser::Parameter("", 'C', "The list of color of the regions to plot separated by commas. All regions will be selected if ommited.");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["mapFile"] = ArgParser::PositionalParameter("Path to the map of regions (i.e. each one must have a different color).");
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file for computing stats", 1);
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch ( const invalid_argument& error)
	{
		cerr<<"Error : "<<error.what()<<endl;
		cerr<<args.help_message(argv[0])<<endl;
		return EXIT_FAILURE;
	}
	
	// We setup the output directory
	string outputDirectory;
	string outputFile = args["output"];
	if (isDir(outputFile))
	{
		outputDirectory = outputFile;
		filenamePrefix = makePath(outputDirectory, stripPath(stripSuffix(args["mapFile"])));
		outputFile += "csv";
	}
	else
	{
		filenamePrefix = stripPath(stripSuffix(outputFile));
		outputDirectory = getPath(outputFile);
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	// We parse the colors of the regions to plot
	set<ColorType> colors;
	if(args["colors"].is_set())
	{
		vector<ColorType> tmp = toVector<ColorType>(args["colors"]);
		colors.insert(tmp.begin(),tmp.end());
	}
	
	// We read the map
	ColorMap* colorizedMap = getColorMapFromFile(args["mapFile"]);
	
	// We erase any colors that is not to be kept
	if(colors.size() > 0)
	{
		for(unsigned j = 0; j < colorizedMap->NumberPixels(); ++j)
		{
			if (colorizedMap->pixel(j) != colorizedMap->null() && colors.count(colorizedMap->pixel(j)) == 0)
				colorizedMap->pixel(j) = colorizedMap->null();
		}
		#if defined DEBUG
		colorizedMap->writeFits(filenamePrefix + "cleaned." +  stripPath(args["mapFile"]) );
		#endif
	}
	
	// We apply the arealimit if any
	if(args["areaLimit"].is_set())
	{
		colorizedMap->preprocessing(args["areaLimit"]);
		#if defined DEBUG || defined WRITE_LIMITED_MAP
			colorizedMap->writeFits(filenamePrefix + "limited." +  stripPath(args["mapFile"]), FitsFile::compress);
		#endif
	}
	
	// We open the output file
	ofstream csvFile(outputFile.c_str(), ios_base::trunc);
	csvFile<<setiosflags(ios::fixed);
	
	// We get the regions
	vector<Region*> regions = getRegions(colorizedMap);
	
	if(regions.empty())
	{
		cerr<<"Warning: No regions found in file "<<args["mapFile"]<<endl;
	}
	else
	{
		string separator = args["separator"];
		FitsFile fitsFile;
		if(args["append"])
		{
			fitsFile.open(args["mapFile"], FitsFile::update);
			if(args["getRegionsInfo"] && ! fitsFile.has("Regions"))
			{
				// We write the Regions into the fits
				fitsFile.writeTable("Regions");
				writeRegions(fitsFile, regions);
			}
		}
		bool wroteHeader = false;
		deque<string> imagesFilenames = args.RemainingPositionalArguments();
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			// We expand the name of the background fits image with the header of the inputImage
			string imageFilename = colorizedMap->getHeader().expand(imagesFilenames[p]);
		
			if(! isFile(imageFilename))
			{
				cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
				continue;
			}
		
			EUVImage* image = getImageFromFile("Unknown", imageFilename);
		
			// We apply the preprocessing
			image->preprocessing(args["statsPreprocessing"]);
			#if defined DEBUG
			image->writeFits(makePath(outputDirectory, stripPath(stripSuffix(imageFilename)) + "preprocessed.fits"));
			#endif
		
			// We transform the image to align it with the colorizedMap
			if(args["registerImages"])
			{
				image->align(colorizedMap);
				#if defined DEBUG
				image->writeFits(makePath(outputDirectory, stripPath(stripSuffix(imageFilename)) + "registered.fits"));
				#endif
			}
		
			#if defined VERBOSE
			cout<<"Computing region statistics for file "<<stripPath(imageFilename)<<endl;
			#endif
		
			if(args["totalStats"])
			{
				// We get the total regions stats
				vector<SegmentationStats*> regions_stats = getTotalRegionStats(colorizedMap, image);
				// We write the header
				if(!wroteHeader && regions_stats.size() > 0)
				{
					csvFile<<"Channel"<<separator<<regions_stats[0]->toString(separator, true)<<endl;
					wroteHeader = true;
				}
				// We write the RegionStats into the fits file
				if(args["append"])
				{
					fitsFile.writeTable(image->Channel()+"_TotalRegionStats");
					writeRegions(fitsFile, regions_stats);
				}
				// We write the RegionStats to the csv
				for (unsigned r = 0; r < regions_stats.size(); ++r)
				{
					csvFile<<image->Channel()<<separator<<regions_stats[r]->toString(separator)<<endl;
					delete regions_stats[r];
				}
			}
			else
			{
				// We get the regions stats
				vector<RegionStats*> regions_stats = getRegionStats(colorizedMap, image, regions);
				// We write the header
				if(!wroteHeader && regions_stats.size() > 0)
				{
					if(args["getRegionsInfo"])
						csvFile<<regions[0]->toString(separator, true)<<separator<<"Channel"<<separator<<regions_stats[0]->toString(separator, true)<<endl;
					else
						csvFile<<"Channel"<<separator<<regions_stats[0]->toString(separator, true)<<endl;
					wroteHeader = true;
				}
				// We write the RegionStats into the fits file
				if(args["append"])
				{
					fitsFile.writeTable(image->Channel()+"_RegionStats");
					writeRegions(fitsFile, regions_stats);
				}
				// We write the RegionStats to the csv
				for (unsigned r = 0; r < regions_stats.size() && r < regions.size(); ++r)
				{
					csvFile<<regions[r]->toString(separator)<<separator<<image->Channel()<<separator<<regions_stats[r]->toString(separator)<<endl;
					delete regions_stats[r];
				}
			}
			delete image;
		}
		if(!fitsFile.isClosed())
			fitsFile.close();
	}
	
	// We cleanup
	csvFile.close();
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		delete regions[r];
	}
	regions.clear();
	delete colorizedMap;
	return EXIT_SUCCESS;
}



