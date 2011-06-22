//! Program that computes the regions statistics of sun images
/*!
@page get_region_stats get_region_stats.x

 This program takes a colorized map in fits format as a mask of regions, and computes different statistics on the sun images provided
 
 The sun images should be similar to the colorized map.
 
 @section usage Usage
 
 <tt> get_region_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_region_stats.x [-option optionvalue, ...] -M colorizeMap fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one colorized map.
 But you can provide as many sun images as desired.
 

@param imageType	The type of the images.
<BR>Possible values are : 
 - EIT
 - EUVI
 - AIA
 - SWAP
 - HMI

@param colorizedMap	A colorized Map of regions (i.e. each one must have a different color).

@param separator	The separator to put between columns.

@param stats	The kind of stats to generate.
<BR>Possible values :
 - A (Active %Region)
 - C (Coronal Hole)
 - R (Regular)

@param regionStatsRadiusRatio	The ratio of the radius of the sun that will be used for the region stats.

@param regionStatsPreprocessing	The steps of preprocessing to apply to the sun images
<BR>Possible values :
 - NAR (Nullify above radius)
 - ALC (Annulus Limb Correction)
 - DivMedian (Division by the median)
 - TakeSqrt (Take the square root)
 - TakeLog (Take the log)
 - DivMode (Division by the mode)
 - DivExpTime (Division by the Exposure Time)

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"


#include "../classes/RegionStats.h"
#include "../classes/ActiveRegionStats.h"
#include "../classes/CoronalHoleStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"


using namespace std;
using namespace dsr;

string filenamePrefix;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// Program version
	const char * version = "2.0";
	
	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	double regionStatsRadiusRatio = 0.95;
	string regionStatsPreprocessing = "NAR";
	
	// Options for the type of coordinate
	string coordinateType;

	// The map of colored regions
	string colorizedMapFileName;
	
	// Option for the output
	string separator = "\t";
	string desiredStats;
	
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP, HMI\n\t", imageType);
	arguments.new_named_double('R', "regionStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the region stats.\n\t",regionStatsRadiusRatio);
	arguments.new_named_string('G', "regionStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t",regionStatsPreprocessing);
	arguments.new_named_string('M',"colorizedMap","file name", "\n\tA colorized Map of regions (i.e. each one must have a different color).\n\t", colorizedMapFileName);
	arguments.new_named_string('C', "coordinateType", "string", "\n\tThe type of coordinates to output positions.\n\tPossible values are : HGS, HGC, HPC, HPR, HCC, HCR\n\t", coordinateType);
	arguments.new_named_string('s', "separator", "string", "\n\tThe separator to put between columns.\n\t", separator);
	arguments.new_named_string('S', "stats", "comma separated list of char (no spaces)", "\n\tThe kind of stats to generate.\n\tPossible values :\n\t\tA (Active Region)\n\t\tC (Coronal Hole)\n\t\tR (Regular)\n\t", desiredStats);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "The name of the fits files containing the images of the sun.", imagesFilenames);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version);
	arguments.process(argc, argv);


	if(imagesFilenames.size() < 1)
	{
		cerr<<"No fits image file given as parameter!"<<endl;
		return EXIT_FAILURE;
	}
		
		
	bool getARStats = (desiredStats.find_first_of("Aa")!=string::npos);
	bool getCHStats = (desiredStats.find_first_of("Cc")!=string::npos);
	bool getRegularStats = (desiredStats.find_first_of("Rr")!=string::npos);
		
	ColorMap* colorizedMap = getImageFromFile(colorizedMapFileName);
	colorizedMap->nullifyAboveRadius(1);
	Coordinate sunCenter = colorizedMap->SunCenter();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
	
		//We read and preprocess the sun image
		string imageFilename = expand(imagesFilenames[p], colorizedMap->header);
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		EUVImage* image = getImageFromFile(imageType, imageFilename);
		image->recenter(sunCenter);
		image->preprocessing(regionStatsPreprocessing, regionStatsRadiusRatio);
		#if DEBUG >= 2
		image->writeFits(filenamePrefix + "preprocessed." +  stripPath(imageFilename) );
		#endif
	
		if(!colorizedMap->checkSimilar(image))
		{
			cerr<<"Warning: image "<<imageFilename<<" is not similar to the colorizedMap!"<<endl;
		}
		
		if(getARStats)
		{
			// We get the regions stats and output them
			vector<ActiveRegionStats*> regions = getActiveRegionStats(colorizedMap, image);
			cout<<"ActiveRegion statistics for file "<<stripPath(imageFilename)<<endl;
			if(regions.size() > 0)
				cout<<regions[0]->toString(separator, true)<<endl;
			else
				cout<<"Empty"<<endl;
			for (unsigned r = 0; r < regions.size(); ++r)
			{
				cout<<regions[r]->toString(separator)<<endl;
				delete regions[r];
			}
		}
		if(getCHStats)
		{
			// We get the regions stats and output them
			vector<CoronalHoleStats*> regions = getCoronalHoleStats(colorizedMap, image);
			cout<<"CoronalHole statistics for file "<<stripPath(imageFilename)<<endl;
			if(regions.size() > 0)
				cout<<regions[0]->toString(separator, true)<<endl;
			else
				cout<<"Empty"<<endl;
			for (unsigned r = 0; r < regions.size(); ++r)
			{
				cout<<regions[r]->toString(separator)<<endl;
				delete regions[r];
			}
		}
		if(getRegularStats)
		{
			// We get the regions stats and output them
			vector<RegionStats*> regions = getRegionStats(colorizedMap, image);
			cout<<"Region statistics for file "<<stripPath(imageFilename)<<endl;
			if(regions.size() > 0)
				cout<<regions[0]->toString(separator, true)<<endl;
			else
				cout<<"Empty"<<endl;
			for (unsigned r = 0; r < regions.size(); ++r)
			{
				cout<<regions[r]->toString(separator)<<endl;
				delete regions[r];
			}
		}
		delete image;
	}
	delete colorizedMap;
	return EXIT_SUCCESS;
	
}


