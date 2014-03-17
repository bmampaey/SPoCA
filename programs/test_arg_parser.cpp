#include <iostream>
#include <string>
#include <vector>

#include "../classes/constants.h"
#include "../classes/tools.h"
#include "../classes/ArgParser.h"
#include "../classes/HistogramPCMClassifier.h"

using namespace std;

string filenamePrefix;

template <typename T>
void test(T param[])
{
	cout<<sizeof(param);
}

int main(int argc, const char **argv)
{

	// We declare our program description
	string programDescription = "This Program does classification and segmentation.\n";
	programDescription+="Version: 3.0\n";
	programDescription+="Author: Benjamin Mampaey, benjamin.mampaey@sidc.be\n";
	
	programDescription+="Compiled on "  __DATE__  " with options :";
	programDescription+="\nNUMBERCHANNELS: " + toString(NUMBERCHANNELS);
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\VERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	// We define our program parameters
	ArgParser args(programDescription);
	
	args("segmentation") = Classifier::segmentationParameters();
	args("classification") = Classifier::classificationParameters();
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["type"] = ArgParser::Parameter("FCM", 'T', "The type of classifier to use for the classification. Possible values are : FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2, HFCM(Histogram FCM), HPFCM(Histogram PFCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)");
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images. Possible values are : EIT, EUVI, AIA, SWAP");
	args["imagePreprocessing"] = ArgParser::Parameter("", 'P', "The steps of preprocessing to apply to the sun images before classification. Can be any combination of the following: NAR=zz.z (Nullify pixels above zz.z*radius); ALC (Annulus Limb Correction); DivMedian (Division by the median); TakeSqrt (Take the square root); TakeLog (Take the log); DivMode (Division by the mode); DivExpTime (Division by the Exposure Time); ThrMin=zz.z (Threshold intensities to minimum zz.z); ThrMax=zz.z (Threshold intensities to maximum zz.z); ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile); ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile); Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["centersFile"] = ArgParser::Parameter('c', "The name of the file containing the centers. If it it not provided the centers will be initialized randomly.");
	args["numberPreviousCenters"] = ArgParser::Parameter(0, 'n', "The number of previous centers to take into account for the median computation of final centers.");
	args["maps"] = ArgParser::Parameter(true, 'M', "Set to false if you don't want to write the segmentation map.");
	args["stats"] = ArgParser::Parameter(false, 's', "Set to compute stats about the generated maps.");
	args["statsPreprocessing"] = ArgParser::Parameter("", 'p', "The steps of preprocessing to apply to the sun images before computing stats. Can be any combination of the following: NAR=zz.z (Nullify pixels above zz.z*radius); ALC (Annulus Limb Correction); DivMedian (Division by the median); TakeSqrt (Take the square root); TakeLog (Take the log); DivMode (Division by the mode); DivExpTime (Division by the Exposure Time); ThrMin=zz.z (Threshold intensities to minimum zz.z); ThrMax=zz.z (Threshold intensities to maximum zz.z); ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile); ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile); Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["output"] = ArgParser::Parameter(".", 'O', "The prefix path for the output files (will be set automatically if directory or left void).");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
	args["chaincodeMinPoints"] = ArgParser::Parameter(4, 'x', "The minimal number of points in a chaincode.");
	args["chaincodeMaxPoints"] = ArgParser::Parameter(0, 'X', "The maximal number of points in a chaincode.");
	args["chaincodeMaxDeviation"] = ArgParser::Parameter(0, 'd', "The maximal deviation of the chaincode curve between 2 points, in arcsec.");
	
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file", NUMBERCHANNELS, NUMBERCHANNELS);

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
	cout<<args("classification")["binSize"]<<endl;
	HistogramFCMClassifier G(5, 10, 2, 300, 7);
	HistogramFCMClassifier F(args("classification"));
}
