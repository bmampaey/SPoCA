//! This Program computes STAFF stats for EUV images.
/*!
@page get_STAFF_stats get_STAFF_stats.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/get_STAFF_stats.x [-option optionvalue ...]  CHSegmentedMap ARSegmentedMap fitsFile [ fitsFile ... ] </tt>

@param CHSegmentedMap	Path to the segmentation map for the Coronal Hole class.
@param ARSegmentedMap	Path to the segmentation map for the Active Region class.
@param fitsFile	Path to a fits file for computing stats

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param ARClass	The color corresponding to the AR class in the ARSegmentedMap.

@param CHClass	The color corresponding to the CH class in the CHSegmentedMap.

@param imageType	The type of the images.
<BR>Possible values are : EIT, EUVI, AIA, SWAP

@param output	The name for the output file or of a directory.

@param separator	The separator to put between columns in the csv file.

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

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <string>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"


#include "../classes/STAFFStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"

using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program computes STAFF stats for EUV images.";
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
	
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images.\nPossible values are : EIT, EUVI, AIA, SWAP");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=3", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["separator"] = ArgParser::Parameter(',', 's', "The separator to put between columns in the csv file.");
	args["CHClass"] = ArgParser::Parameter('c', "The color corresponding to the CH class in the CHSegmentedMap.");
	args["ARClass"] = ArgParser::Parameter('a', "The color corresponding to the AR class in the ARSegmentedMap.");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["CHSegmentedMap"] = ArgParser::PositionalParameter("Path to the segmentation map for the Coronal Hole class.");
	args["ARSegmentedMap"] = ArgParser::PositionalParameter("Path to the segmentation map for the Active Region class.");
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
	
	// We check the arguments
	if(! args["CHClass"].is_set())
	{
		cerr<<"Error: You need to specify the Coronal Hole class!"<<endl;
		return EXIT_FAILURE;
	}
	
	if(! args["ARClass"].is_set())
	{
		cerr<<"Error: You need to specify the Active Region class!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We check if the output is a directory
	string outputDirectory = args["output"], outputFileName;
	if (isDir(outputDirectory))
	{
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(args["CHSegmentedMap"])) + "_and_" + stripSuffix(stripPath(args["ARSegmentedMap"])) + ".";
		outputFileName = outputDirectory + "/" + stripSuffix(stripPath(args["CHSegmentedMap"])) + "_and_" + stripSuffix(stripPath(args["ARSegmentedMap"])) + "_on_";
	}
	else
	{
		outputDirectory = getPath(args["output"]);
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(args["CHSegmentedMap"])) + "_and_" + stripSuffix(stripPath(args["ARSegmentedMap"])) + ".";
		outputFileName = args["output"] + "_on_";
		
		// We check if the outputDirectory exists
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	// We read the CHSegmentedMap
	ColorMap* CHMap_ondisk = getColorMapFromFile(args["CHSegmentedMap"]);
	
	// If the ARSegmentedMap is different from the CHSegmentedMap, we read it and check if they are similar
	ColorMap* ARMap_ondisk = CHMap_ondisk;
	if(args["CHSegmentedMap"].as<string>() != args["ARSegmentedMap"].as<string>())
	{
		ARMap_ondisk = getColorMapFromFile(args["ARSegmentedMap"]);
		ARMap_ondisk->align(CHMap_ondisk);
	}

	// We construct a total (on-disk + off-disk) copy of ARMap_ondisk
	ColorMap* ARMap_total = new ColorMap(ARMap_ondisk);
	
	// We restrict ARMap_ondisk and CHMap_ondisk to the disk
	ARMap_ondisk->nullifyAboveRadius();
	CHMap_ondisk->nullifyAboveRadius();
	
	#if defined DEBUG
	CHMap_ondisk->writeFits(filenamePrefix + "ondisk." +  stripPath(args["CHSegmentedMap"]), FitsFile::compress);
	if(args["CHSegmentedMap"].as<string>() != args["ARSegmentedMap"].as<string>())
		ARMap_ondisk->writeFits(filenamePrefix + "ondisk." +  stripPath(args["ARSegmentedMap"]), FitsFile::compress);
	#endif
	
	// We construct a CHmap limited to 15 deg longitude
	ColorMap* CHMap_limited = new ColorMap(CHMap_ondisk);
	CHMap_limited->nullifyAboveLongLat(15);
	
	#if defined DEBUG
	CHMap_limited->writeFits(filenamePrefix + "limited." +  stripPath(args["CHSegmentedMap"]), FitsFile::compress);
	#endif
	
	string separator = args["separator"];
	
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the sun image 
		string imageFilename = CHMap_ondisk->getHeader().expand(imagesFilenames[p]);
		if(! isFile(imageFilename))
		{
			cerr<<"Error : Could not find "<<imageFilename<<"!"<<endl;
			continue;
		}
		EUVImage* image = getImageFromFile(args["imageType"], imageFilename);
		
		// We align it with the Segmented maps and check if they are similar
		image->align(CHMap_ondisk);
		string dissimilarity = checkSimilar(CHMap_ondisk, image);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imageFilename<<" and the CHSegmentedMap "<<args["CHSegmentedMap"]<<" are not similar: "<<dissimilarity<<endl;
		}
		
		// We apply the intensities preprocessing
		image->preprocessing(args["statsPreprocessing"]);

		// We open the output file
		ofstream outputFile((outputFileName + stripSuffix(stripPath(imageFilename)) + ".csv").c_str(), ios_base::trunc);
		outputFile<<setiosflags(ios::fixed);
		
		// We extract the AR STAFF stats on the whole image
		STAFFStats AR_staff_stats = getSTAFFStats(ARMap_total, args["ARClass"], image);
		outputFile<<"Channel"<<separator<<"Type"<<separator<<AR_staff_stats.toString(separator, true)<<endl;
		outputFile<<image->Channel()<<separator<<"AR_all"<<separator<<AR_staff_stats.toString(separator)<<endl;
		
		// We extract the CH STAFF stats on the limited image
		STAFFStats CH_staff_stats = getSTAFFStats(CHMap_limited, args["CHClass"], image);
		outputFile<<image->Channel()<<separator<<"CH_central_meridian"<<separator<<CH_staff_stats.toString(separator)<<endl;
		
		// We extract the STAFF stats on the disc
		vector<STAFFStats> staff_stats = getSTAFFStats(CHMap_ondisk, args["CHClass"], ARMap_ondisk, args["ARClass"], image);
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
	delete CHMap_limited;
	return EXIT_SUCCESS;
}




