//! Program that computes the STAFF statistics of maps
/*!
@page get_STAFF_stats get_STAFF_stats.x

 This program takes a colorized map in fits format as a mask of regions, and computes different statistics on the sun images provided
 
 The sun images should be similar to the colorized map.
 
 @section usage Usage
 
 <tt> get_STAFF_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_STAFF_stats.x [-option optionvalue, ...] -M colorizeMap fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one colorized map.
 But you can provide as many sun images as desired.
 

@param imageType	The type of the images.
<BR>Possible values are : 
 - EIT
 - EUVI
 - AIA
 - SWAP
 - HMI

@param CHMap	A colorized Map of regions (i.e. each one must have a different color).

@param separator	The separator to put between columns.

@param intensitiesStatsRadiusRatio	The ratio of the radius of the sun that will be used for the region stats.

@param intensitiesStatsPreprocessing	The steps of preprocessing to apply to the sun images
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


#include "../classes/STAFFStats.h"
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
	double intensitiesStatsRadiusRatio = 1;
	string intensitiesStatsPreprocessing = "NAR";

	// The Segmented Maps and corresponding classes color
	string CHSegmentedMap, ARSegmentedMap;
	ColorType CHClass, ARClass;
	
	// Option for the output
	string separator = "\t";
	bool append = false;
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP, HMI\n\t", imageType);
	arguments.new_flag('a', "append", "\n\tSet this flag if you want append a new table in the fitsfile with the region stats.\n\t", append);
	arguments.new_named_double('R', "intensitiesStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the region stats.\n\t",intensitiesStatsRadiusRatio);
	arguments.new_named_string('G', "intensitiesStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t",intensitiesStatsPreprocessing);
	arguments.new_named_unsigned_int('c', "CHClass", "unsigned integer", "\n\tThe color corresponding to the CH class in the CHSegmentedMap.\n\t", CHClass);
	arguments.new_named_string('C', "CHSegmentedMap", "string", "\n\tA segmented map for the Coronal Hole class.\n\t", CHSegmentedMap);
	arguments.new_named_unsigned_int('a', "ARClass", "unsigned integer", "\n\tThe color corresponding to the AR class in the ARSegmentedMap.\n\t", ARClass);
	arguments.new_named_string('A', "ARSegmentedMap", "string", "\n\tA segmented map for the Active Region class.\n\t", ARSegmentedMap);

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
	
	// We read the CHSegmentedMap
	ColorMap* CHMap = getImageFromFile(CHSegmentedMap);
	// If the ARSegmentedMap is different from the CHSegmentedMap, we read it and check if they are similar
	ColorMap* ARMap = CHMap;
	if(CHSegmentedMap != ARSegmentedMap)
	{
		ARMap = getImageFromFile(ARSegmentedMap);
		ARMap->recenter(CHMap->SunCenter());
		
		string dissimilarity = checkSimilar(CHMap, ARMap);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: "<<CHSegmentedMap<<" and "<<ARSegmentedMap<<" are not similar: "<<dissimilarity<<endl;
		}
	}
	
	RealPixLoc sunCenter = CHMap->SunCenter();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the sun image 
		string imageFilename = expand(imagesFilenames[p], CHMap->getHeader());
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		EUVImage* image = getImageFromFile(imageType, imageFilename);
		
		// We align it with the Segmented maps and check if they are similar
		image->recenter(sunCenter);
		string dissimilarity = checkSimilar(CHMap, image);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imageFilename<<" and the CHSegmentedMap "<<CHSegmentedMap<<" are not similar: "<<dissimilarity<<endl;
		}
		
		// We extract the STAFF stats on the whole image
		STAFFStats AR_staff_stats = getSTAFFStats(ARMap, ARClass, image);
		cout<<image->Channel()<<"AR_all"<<AR_staff_stats.toString(separator)<<endl;
		
		// We extract the STAFF stats on the disc
		image->preprocessing(intensitiesStatsPreprocessing, intensitiesStatsRadiusRatio);
		vector<STAFFStats> staff_stats = getSTAFFStats(CHMap, CHClass, ARMap, ARClass, image);
		cout<<image->Channel()<<separator<<"CH_ondisc"<<separator<<staff_stats[0].toString(separator)<<endl;
		cout<<image->Channel()<<separator<<"AR_ondisc"<<separator<<staff_stats[1].toString(separator)<<endl;
		cout<<image->Channel()<<separator<<"QS_ondisc"<<separator<<staff_stats[2].toString(separator)<<endl;
		delete image;
	}
	
	if (ARMap != CHMap)
		delete ARMap;
	delete CHMap;
	return EXIT_SUCCESS;
}



