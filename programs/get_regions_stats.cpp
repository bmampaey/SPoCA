//! Program that computes the regions statistics of sun images
/*!
@page get_regions_stats get_regions_stats.x

 This program takes a colorized map in fits format as a mask of regions, and computes different statistics on the sun images provided
 
 The sun images should be similar to the colorized map.
 
 @section usage Usage
 
 <tt> get_regions_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_regions_stats.x [-option optionvalue, ...] -M colorizeMap fitsFileName1 fitsFileName2 </tt>
 
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
 - ThrMinzz.z (Threshold intensities to minimum the zz.z percentile) 
 - ThrMaxzz.z (Threshold intensities to maximum the zz.z percentile)
 - Smoothzz.z Binomial smoothing of zz.z arcsec

@param colors The list of colors to select separated by commas (no spaces)
<BR>All colors will be selected if ommited.

@param colorsFilename A file containing a list of colors to select separated by commas
<BR>All colors will be selected if ommited.

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
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"


using namespace std;
using namespace dsr;

string filenamePrefix;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// Program version
	string version = "2.0";
	
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
	
	// Options for the colors to select
	string colorsString;
	string colorsFilename;
	
	// Option for the output
	string separator = "\t";
	bool getRegionsInfo = false;
	bool append = false;
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP, HMI\n\t", imageType);
	arguments.new_flag('r', "getRegionsInfo", "\n\tSet this flag if you want also to get region information.\n\t", getRegionsInfo);
	arguments.new_flag('a', "append", "\n\tSet this flag if you want append a new table in the fitsfile with the region stats.\n\t", append);
	arguments.new_named_double('R', "regionStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the region stats.\n\t",regionStatsRadiusRatio);
	arguments.new_named_string('G', "regionStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t",regionStatsPreprocessing);
	arguments.new_named_string('M',"colorizedMap","file name", "\n\tA colorized Map of regions (i.e. each one must have a different color).\n\t", colorizedMapFileName);
	arguments.new_named_string('T', "coordinateType", "string", "\n\tThe type of coordinates to output positions.\n\tPossible values are : HGS, HGC, HPC, HPR, HCC, HCR\n\t", coordinateType);
	arguments.new_named_string('s', "separator", "string", "\n\tThe separator to put between columns.\n\t", separator);
	arguments.new_named_string('c', "colors", "string", "\n\tThe list of colors to select separated by commas (no spaces)\n\tAll colors will be selected if ommited.\n\t", colorsString);
	arguments.new_named_string('C', "colorsFilename", "string", "\n\tA file containing a list of colors to select separated by commas\n\tAll colors will be selected if ommited.\n\t", colorsFilename);

	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "The name of the fits files containing the images of the sun.", imagesFilenames);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version.c_str());
	arguments.process(argc, argv);


	if(imagesFilenames.size() < 1)
	{
		cerr<<"No fits image file given as parameter!"<<endl;
		return EXIT_FAILURE;
	}
		
	ColorMap* colorizedMap = getImageFromFile(colorizedMapFileName);
	RealPixLoc sunCenter = colorizedMap->SunCenter();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		//We read and preprocess the sun image
		string imageFilename = expand(imagesFilenames[p], colorizedMap->getHeader());
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
		
		// We check if the images are similars
		string dissimilarity = checkSimilar(colorizedMap, image);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imageFilename<<" and the colorizedMap "<<colorizedMapFileName<<" are not similar: "<<dissimilarity<<endl;
		}
		
		cout<<"Region statistics for file "<<stripPath(imageFilename)<<endl;

		if(getRegionsInfo)
		{
			// We get the regions
			vector<Region*> regions = getRegions(colorizedMap);
			// We get the regions stats
			vector<RegionStats*> regions_stats = getRegionStats(colorizedMap, image, regions);
			// We output them both
			if(regions.size() > 0 && regions_stats.size() > 0)
				cout<<regions[0]->toString(separator, true)<<separator<<regions_stats[0]->toString(separator, true)<<endl;
			else
				cout<<"Empty"<<endl;
			
			if(append)
			{
				FitsFile file(colorizedMapFileName, FitsFile::update);
				if (! file.has("Regions"))
				{
					// We write the Regions into the fits
					file.writeTable("Regions");
					writeRegions(file, regions);
				}
				
				// We write the RegionStats into the fits
				file.writeTable(image->Instrument()+"RegionStats");
				writeRegions(file, regions_stats);
			}
			for (unsigned r = 0; r < regions_stats.size() && r < regions.size(); ++r)
			{
				cout<<regions[r]->toString(separator)<<separator<<regions_stats[r]->toString(separator)<<endl;
				delete regions_stats[r];
				delete regions[r];
			}
		}
		else
		{
			// We get only the regions stats and output them
			vector<RegionStats*> regions_stats = getRegionStats(colorizedMap, image);
			if(regions_stats.size() > 0)
				cout<<regions_stats[0]->toString(separator, true)<<endl;
			else
				cout<<"Empty"<<endl;
			if(append)
			{
				FitsFile file(colorizedMapFileName, FitsFile::update);
				// We write the RegionStats into the fits
				file.writeTable(image->Instrument()+"RegionStats");
				writeRegions(file, regions_stats);
			}
			for (unsigned r = 0; r < regions_stats.size(); ++r)
			{
				cout<<regions_stats[r]->toString(separator)<<endl;
				delete regions_stats[r];
			}
		}
		delete image;
	}
	delete colorizedMap;
	return EXIT_SUCCESS;
	
}



