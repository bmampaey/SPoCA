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
	
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images.\nPossible values are : EIT, EUVI, AIA, SWAP");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images to the map.");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=0.95", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
	args["chaincodeMinPoints"] = ArgParser::Parameter(4, 'x', "The minimal number of points in a chaincode.");
	args["chaincodeMaxPoints"] = ArgParser::Parameter(0, 'X', "The maximal number of points in a chaincode.");
	args["chaincodeMaxDeviation"] = ArgParser::Parameter(0, 'd', "The maximal deviation of the chaincode curve between 2 points, in arcsec.");
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
		EUVImage* image = getImageFromFile(args["imageType"], imagesFilenames[p]);
		image->preprocessing(args["statsPreprocessing"]);
		if(args["registerImages"])
		{
			image->align(segmentedMap);
		}
		#if defined DEBUG
			image->getHeader().set("IPREPROC", args["statsPreprocessing"], "Image Preprocessing");
			image->writeFits(outputDirectory + "/" + stripPath(stripSuffix(imagesFilenames[p])) + ".preprocessed.fits");
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
	writeARMap(segmentedMap, outputFile, images, !args["uncompressed"], args["chaincodeMinPoints"], args["chaincodeMaxPoints"], args["chaincodeMaxDeviation"]);
	
	// We cleanup
	delete segmentedMap;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
	
	return EXIT_SUCCESS;
}
