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

@param CHSegmentedMap	A segmented map for the Coronal Hole class.
@param CHClass The color corresponding to the CH class in the CHSegmentedMap.

@param ARSegmentedMap	A segmented map for the Active Region class.
@param ARClass The color corresponding to the AR class in the ARSegmentedMap.

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
	double intensitiesStatsRadiusRatio = 3;
	string intensitiesStatsPreprocessing = "NAR";

	// The Segmented Maps and corresponding classes color
	string CHSegmentedMap, ARSegmentedMap;
	ColorType CHClass, ARClass;
	
	// Option for the output
	string separator = "\t";
	
	// Option for the output file/directory
	string output = ".";
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
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

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP, HMI\n\t", imageType);
	arguments.new_named_double('R', "intensitiesStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the region stats.\n\t",intensitiesStatsRadiusRatio);
	arguments.new_named_string('G', "intensitiesStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t",intensitiesStatsPreprocessing);
	arguments.new_named_string('s', "separator", "string", "\n\tThe separator to put between columns.\n\t", separator);
	arguments.new_named_unsigned_int('c', "CHClass", "unsigned integer", "\n\tThe color corresponding to the CH class in the CHSegmentedMap.\n\t", CHClass);
	arguments.new_named_string('C', "CHSegmentedMap", "string", "\n\tA segmented map for the Coronal Hole class.\n\t", CHSegmentedMap);
	arguments.new_named_unsigned_int('a', "ARClass", "unsigned integer", "\n\tThe color corresponding to the AR class in the ARSegmentedMap.\n\t", ARClass);
	arguments.new_named_string('A', "ARSegmentedMap", "string", "\n\tA segmented map for the Active Region class.\n\t", ARSegmentedMap);
	arguments.new_named_string('O', "output","file/directory name", "\n\tThe name for the output file/directory.\n\t", output);
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
	
	
	// We check if the output is a directory
	string outputDirectory, outputFileName;
	if (isDir(output))
	{
		outputDirectory = output;
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(CHSegmentedMap)) + "_and_" + stripSuffix(stripPath(ARSegmentedMap)) + ".";
		outputFileName = outputDirectory + "/" + stripSuffix(stripPath(CHSegmentedMap)) + "_and_" + stripSuffix(stripPath(ARSegmentedMap)) + "_on_";
	}
	else
	{
		outputDirectory = getPath(output);
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(CHSegmentedMap)) + "_and_" + stripSuffix(stripPath(ARSegmentedMap)) + ".";
		outputFileName = output + "_on_";
		
		// We check if the outputDirectory exists
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	// We read the CHSegmentedMap
	ColorMap* CHMap_ondisk = getImageFromFile(CHSegmentedMap);
	
	// We will align all files on the suncenter of the CHSegmentedMap
	RealPixLoc sunCenter = CHMap_ondisk->SunCenter();
	
	// If the ARSegmentedMap is different from the CHSegmentedMap, we read it and check if they are similar
	ColorMap* ARMap_ondisk = CHMap_ondisk;
	if(CHSegmentedMap != ARSegmentedMap)
	{
		ARMap_ondisk = getImageFromFile(ARSegmentedMap);
		ARMap_ondisk->recenter(sunCenter);
		
		string dissimilarity = checkSimilar(CHMap_ondisk, ARMap_ondisk);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: "<<CHSegmentedMap<<" and "<<ARSegmentedMap<<" are not similar: "<<dissimilarity<<endl;
		}
	}

	// We construct a total (on-disk + off-disk) copy of ARMap_ondisk
	ColorMap* ARMap_total = new ColorMap(ARMap_ondisk);
	
	// We restrict ARMap_ondisk and CHMap_ondisk to the disk
	ARMap_ondisk->nullifyAboveRadius();
	CHMap_ondisk->nullifyAboveRadius();
	
	#if defined DEBUG
	CHMap_ondisk->writeFits(filenamePrefix + "ondisk." +  stripPath(CHSegmentedMap), FitsFile::compress);
	if(CHSegmentedMap != ARSegmentedMap)
		ARMap_ondisk->writeFits(filenamePrefix + "ondisk." +  stripPath(ARSegmentedMap), FitsFile::compress);
	#endif
	
	// We construct a CHmap limited to 15 deg longitude
	ColorMap* CHMap_limited = new ColorMap(CHMap_ondisk);
	CHMap_limited->nullifyAboveLongLat(15);
	
	#if defined DEBUG
	CHMap_limited->writeFits(filenamePrefix + "limited." +  stripPath(CHSegmentedMap), FitsFile::compress);
	#endif
	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the sun image 
		string imageFilename = expand(imagesFilenames[p], CHMap_ondisk->getHeader());
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		EUVImage* image = getImageFromFile(imageType, imageFilename);
		
		// We align it with the Segmented maps and check if they are similar
		image->recenter(sunCenter);
		string dissimilarity = checkSimilar(CHMap_ondisk, image);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imageFilename<<" and the CHSegmentedMap "<<CHSegmentedMap<<" are not similar: "<<dissimilarity<<endl;
		}
		
		// We apply the intensities preprocessing
		image->preprocessing(intensitiesStatsPreprocessing, intensitiesStatsRadiusRatio);

		// We open the output file
		ofstream outputFile((outputFileName + stripSuffix(stripPath(imageFilename)) + ".csv").c_str(), ios_base::trunc);
		outputFile<<setiosflags(ios::fixed);
		
		// We extract the AR STAFF stats on the whole image
		STAFFStats AR_staff_stats = getSTAFFStats(ARMap_total, ARClass, image);
		outputFile<<"Channel"<<separator<<"Type"<<separator<<AR_staff_stats.toString(separator, true)<<endl;
		outputFile<<image->Channel()<<separator<<"AR_all"<<separator<<AR_staff_stats.toString(separator)<<endl;
		
		// We extract the CH STAFF stats on the limited image
		STAFFStats CH_staff_stats = getSTAFFStats(CHMap_limited, CHClass, image);
		outputFile<<image->Channel()<<separator<<"CH_central_meridian"<<separator<<CH_staff_stats.toString(separator)<<endl;
		
		// We extract the STAFF stats on the disc
		vector<STAFFStats> staff_stats = getSTAFFStats(CHMap_ondisk, CHClass, ARMap_ondisk, ARClass, image);
		outputFile<<image->Channel()<<separator<<"CH_ondisc"<<separator<<staff_stats[0].toString(separator)<<endl;
		outputFile<<image->Channel()<<separator<<"AR_ondisc"<<separator<<staff_stats[1].toString(separator)<<endl;
		outputFile<<image->Channel()<<separator<<"QS_ondisc"<<separator<<staff_stats[2].toString(separator)<<endl;
		
		outputFile.close();
		delete image;
	}
	
	if (ARMap_ondisk != CHMap_ondisk)
		delete ARMap_ondisk;
	delete CHMap_ondisk;
	delete ARMap_total;

	return EXIT_SUCCESS;
}




