//! Program that computes the segmentations statistics of sun images
/*!
@page get_segmentation_stats get_segmentation_stats.x

 This program takes a segmetation map in fits format, and computes different statistics on the sun images provided
 
 The sun images should be similar to the colorized map.
 
 @section usage Usage
 
 <tt> get_segmentation_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_segmentation_stats.x [-option optionvalue, ...] -M colorizeMap fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one colorized map.
 But you can provide as many sun images as desired.
 

@param imageType	The type of the images.
<BR>Possible values are : 
 - EIT
 - EUVI
 - AIA
 - SWAP
 - HMI

@param colorizedMap	A segmentation map (i.e. each class must have a different color).

@param separator	The separator to put between columns.

@param areaLimitValue	The value for the areaLimitType.

@param areaLimitType	The type of the limit of the area of the map taken into account the computation of stats.
<BR>Possible values :
 - NAR (Nothing above radius)
 - Long (Limit to absolute longitude)
 - Lat (Limit to absolute latitude)

@param intensitiesStatsRadiusRatio	The ratio of the radius of the sun that will be used for the segmentation stats.

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

@param classes The list of classes to select separated by commas (no spaces)
<BR>All classes will be selected if ommited.

@param classesFilename A file containing a list of classes to select separated by commas
<BR>All classes will be selected if ommited.

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


#include "../classes/SegmentationStats.h"
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

	// Options for the preprocessing of maps
	double areaLimitValue = 1.;
	string areaLimitType = "NAR";

	// Options for the preprocessing of intensity images
	double intensitiesStatsRadiusRatio = 0.95;
	string intensitiesStatsPreprocessing = "NAR";

	// The segmented map
	string colorizedMapFileName;
	
	// Options for the classes to select
	string classesString;
	string classesFilename;
	
	// Option for the output
	string separator = "\t";
	bool append = false;
	
	string programDescription = "This Programm output segmentation statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
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
	arguments.new_flag('a', "append", "\n\tSet this flag if you want append a new table in the fitsfile with the segmentation stats.\n\t", append);
	arguments.new_named_double('r', "areaLimitValue", "positive real", "\n\tThe value for the areaLimitType.\n\t",areaLimitValue);
	arguments.new_named_string('g', "areaLimitType", "string", "\n\tThe type of the limit of the area of the map taken into account the computation of stats.\n\tPossible values :\n\t\tNAR\n\t\tLong\n\t\tLat\n\t", areaLimitType);
	arguments.new_named_double('R', "intensitiesStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the intensities stats.\n\t",intensitiesStatsRadiusRatio);
	arguments.new_named_string('G', "intensitiesStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images for the intensities stats.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t",intensitiesStatsPreprocessing);
	arguments.new_named_string('M',"colorizedMap","file name", "\n\tA segmented map (i.e. each class must have a different color).\n\t", colorizedMapFileName);
	arguments.new_named_string('s', "separator", "string", "\n\tThe separator to put between columns.\n\t", separator);
	arguments.new_named_string('c', "classes", "string", "\n\tThe list of classes to select separated by commas (no spaces)\n\tAll classes will be selected if ommited.\n\t", classesString);
	arguments.new_named_string('C', "classesFilename", "string", "\n\tA file containing a list of classes to select separated by commas\n\tAll classes will be selected if ommited.\n\t", classesFilename);

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
	
	vector<ColorType> classes;
	// We parse the colors to overlay
	if(! classesFilename.empty())
	{
		ifstream classesFile(classesFilename.c_str());
		if(classesFile.good())
		{
			classesFile>>classes;
		}
		else
		{
			cerr << "Error reading list of classes from file: "<<classesFilename<<endl;
			return 2;
		}
	}
	if(! classesFilename.empty())
	{
		istringstream ss(classesFilename);
		ss>>classes;
	}
	
	ColorMap* colorizedMap = getImageFromFile(colorizedMapFileName);
	// We apply the arealimit if any
	if(areaLimitType == "NAR")
		colorizedMap->nullifyAboveRadius(areaLimitValue);
	if(areaLimitType == "Long")
		colorizedMap->nullifyAboveLongLat(areaLimitValue);
	if(areaLimitType == "Lat")
		colorizedMap->nullifyAboveLongLat(360, areaLimitValue);
	#if defined DEBUG
	colorizedMap->writeFits(filenamePrefix + "limited." +  stripPath(colorizedMapFileName) );
	#endif
	
	RealPixLoc sunCenter = colorizedMap->SunCenter();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the sun image
		string imageFilename = expand(imagesFilenames[p], colorizedMap->getHeader());
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		
		EUVImage* image = getImageFromFile(imageType, imageFilename);
		
		image->recenter(sunCenter);
		// We preprocess the sun image if necessary
		if(! intensitiesStatsPreprocessing.empty())
			image->preprocessing(intensitiesStatsPreprocessing, intensitiesStatsRadiusRatio);
		
		#if defined DEBUG
		image->writeFits(filenamePrefix + "preprocessed." +  stripPath(imageFilename) );
		#endif
		
		// We check if the images are similars
		string dissimilarity = checkSimilar(colorizedMap, image);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imageFilename<<" and the colorizedMap "<<colorizedMapFileName<<" are not similar: "<<dissimilarity<<endl;
		}
		#if defined VERBOSE
		cout<<"Segmentation statistics for file "<<stripPath(imageFilename)<<endl;
		#endif
		// We get the segmentation stats and output them
		vector<SegmentationStats*> segmentation_stats;
		if(classes.size() > 0)
		{
			segmentation_stats = getSegmentationStats(colorizedMap, image, classes);
		}
		else
		{
			segmentation_stats = getSegmentationStats(colorizedMap, image);
		}
		
		if(segmentation_stats.size() > 0)
			cout<<"Channel"<<separator<<segmentation_stats[0]->toString(separator, true)<<endl;
		else
			cout<<"Empty"<<endl;
		
		if(append)
		{
			FitsFile file(colorizedMapFileName, FitsFile::update);
			// We write the SegmentationStats into the fits
			file.writeTable(image->Channel()+"_SegmentationStats");
			writeRegions(file, segmentation_stats);
		}
		for (unsigned r = 0; r < segmentation_stats.size(); ++r)
		{
			cout<<image->Channel()<<separator<<segmentation_stats[r]->toString(separator)<<endl;
			delete segmentation_stats[r];
		}
		delete image;
	}
	delete colorizedMap;
	return EXIT_SUCCESS;
	
}



