//! This Program extract an Active Region map from a segmentation map.
/*!
@page get_AR_map get_AR_map.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/get_AR_map.x [-option optionvalue ...]  mapFile [ fitsFile ... ] </tt>

@param mapFile	Path to the segmentation map
@param fitsFile	Path to a fits file for computing stats

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param color	The value of the pixels in the segmentation map corresponding to Active Region.

@param imageType	The type of the images.
<BR>Possible values are : EIT, EUVI, AIA, SWAP

@param output	The name for the output file or of a directory.

@param registerImages	Set to register/align the images to the map.

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

@param uncompressed	Set this flag if you want results maps to be uncompressed.

mapPreprocessing parameters:

@param aggregated	Aggregate regions so that one region correspond to only one connected component

@param aggregation	Aggregation factor in arcsec.

@param chaincodeMaxDeviation	The maximal deviation of the chaincode curve between 2 points, in arcsec.

@param chaincodeMaxPoints	The maximal number of points in a chaincode.

@param chaincodeMinPoints	The minimal number of points in a chaincode.

@param cleaning	Cleaning factor in arcsec.

@param minimalSize	Minal size of regions in arcsec². Smaller regions will be discarded

@param projection	Projection used for the aggregation.

@param useRawArea	When discarding small regions, use raw area instead of real area.

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
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"
#include "../classes/ActiveRegion.h"

using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program extract an Active Region map from a segmentation map.";
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
	
	args("mapPreprocessing") = ARMapParameters();
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images.\nPossible values are : EIT, EUVI, AIA, SWAP");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images to the map.");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=0.95", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
	args["color"] = ArgParser::Parameter('c', "The value of the pixels in the segmentation map corresponding to Active Region.");
	args["mapFile"] = ArgParser::PositionalParameter("Path to the segmentation map");
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file for computing stats");
	
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
	if(! args["mapFile"].is_set())
	{
		cerr<<"Error: You need to specify the segmentation map!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We setup the output directory and the filename prefix
	string outputDirectory;
	string outputFile = args["output"];
	if (isDir(outputFile))
	{
		outputDirectory = outputFile;
		filenamePrefix = makePath(outputDirectory, stripPath(stripSuffix(args["mapFile"])));
		filenamePrefix += ".";
		outputFile = filenamePrefix + "ARMap.fits";
	}
	else
	{
		outputDirectory = getPath(outputFile);
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
		filenamePrefix = stripSuffix(outputFile);
	}
	
	// We read the segmentation map
	ColorMap* segmentedMap = getColorMapFromFile(args["mapFile"]);
	
	// We read and preprocess the sun images
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	vector<EUVImage*> images;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We expand the name of the background fits image with the header of the inputImage
		string imageFilename = segmentedMap->getHeader().expand(imagesFilenames[p]);
		
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		
		EUVImage* image = getImageFromFile(args["imageType"], imageFilename);
		
		// We apply the preprocessing
		image->preprocessing(args["statsPreprocessing"]);
		#if defined DEBUG
		image->writeFits(makePath(outputDirectory, stripPath(stripSuffix(imageFilename)) + "preprocessed.fits"));
		#endif
		
		// We transform the image to align it with the segmentedMap
		if(args["registerImages"])
		{
			image->align(segmentedMap);
		}
		#if defined DEBUG
			image->getHeader().set("IPREPROC", args["statsPreprocessing"], "Image Preprocessing");
			image->writeFits(makePath(outputDirectory, stripPath(stripSuffix(imageFilename)) + "registered.fits"));
		#endif
		images.push_back(image);
	}
	
	// We transform the segmented map into a binary map
	if (args["color"].is_set())
	{
		segmentedMap->bitmap(toInt(args["color"]));
	}
	else
	{
		ColorType minColor, maxColor;
		segmentedMap->minmax(minColor, maxColor);
		#if defined VERBOSE
		cout<<"Found color for AR "<<maxColor<<endl;
		#endif
		segmentedMap->bitmap(maxColor);
	}
	#if defined DEBUG
		segmentedMap->writeFits(filenamePrefix + "binary.fits");
	#endif
	
	// And we write the map of AR
	filenamePrefix += "ARmap.";
	writeARMap(segmentedMap, outputFile, images, args("mapPreprocessing"), !args["uncompressed"]);
	
	// We cleanup
	delete segmentedMap;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
	
	return EXIT_SUCCESS;
}
