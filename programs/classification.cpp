//! This Program does classification and segmentation.
/*!
@page classification classification.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/classification.x [-option optionvalue ...]  fitsFile fitsFile </tt>

@param fitsFile	Path to a fits file

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param centersFile	The name of the file containing the centers. If it it not provided the centers will be initialized randomly.

@param imagePreprocessing	The steps of preprocessing to apply to the sun images.
<BR>Can be any combination of the following:
<BR> NAR=zz.z (Nullify pixels above zz.z*radius)
<BR> ALC (Annulus Limb Correction)
<BR> DivMedian (Division by the median)
<BR> TakeSqrt (Take the square root)
<BR> TakeLog (Take the log)
<BR> TakeAbs (Take the absolute value)
<BR> DivMode (Division by the mode)
<BR> DivExpTime (Division by the Exposure Time)
<BR> ThrMin=zz.z (Threshold intensities to minimum zz.z)
<BR> ThrMax=zz.z (Threshold intensities to maximum zz.z)
<BR> ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)
<BR> ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param imageType	The type of the images.
<BR>Possible values: EIT, EUVI, AIA, SWAP

@param map	Set to false if you don't want to write the segmentation map.

@param numberPreviousCenters	The number of previous centers to take into account for the median computation of final centers.

@param output	The name for the output file or of a directory.

@param registerImages	Set to register/align the images when running multi channel classification.

@param stats	Set to compute stats about the generated maps.

@param statsPreprocessing	The steps of preprocessing to apply to the sun images.
<BR>Can be any combination of the following:
<BR> NAR=zz.z (Nullify pixels above zz.z*radius)
<BR> ALC (Annulus Limb Correction)
<BR> DivMedian (Division by the median)
<BR> TakeSqrt (Take the square root)
<BR> TakeLog (Take the log)
<BR> TakeAbs (Take the absolute value)
<BR> DivMode (Division by the mode)
<BR> DivExpTime (Division by the Exposure Time)
<BR> ThrMin=zz.z (Threshold intensities to minimum zz.z)
<BR> ThrMax=zz.z (Threshold intensities to maximum zz.z)
<BR> ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)
<BR> ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param type	The type of classifier to use for the classification.
<BR>Possible values: FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2, HFCM(Histogram FCM), HPFCM(Histogram PFCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)

@param uncompressed	Set this to true if you want results maps to be uncompressed.

classification parameters:

@param FCMfuzzifier	The FCM fuzzifier value. Set if you want to override the global fuzzifier value for FCM.

@param FCMweight	The FCM weight for PFCM classification.

@param PCMfuzzifier	The PCM fuzzifier value. Set if you want to override the global fuzzifier value for PCM.

@param PCMweight	The PCM  weight for PFCM classification.

@param binSize	The size of the bins of the histogram.
<BR>NB : Be carreful that the histogram is built after the image preprocessing.

@param fuzzifier	The fuzzifier value

@param maxNumberIteration	The maximal number of iteration for the classification.

@param neighborhoodRadius	Only for spatial classifiers like SPoCA. The neighborhoodRadius is half the size of the square of neighboors.
<BR>For example with a value of 1, the square has a size of 3x3.

@param numberClasses	The number of classes to classify the sun images into.

@param precision	The precision to be reached to stop the classification.

segmentation parameters:

@param AR	Only for fix segmentation. The classes of the Active Region.

@param CH	Only for fix segmentation. The classes of the Coronal Hole.

@param QS	Only for fix segmentation. The classes of the Quiet Sun.

@param limits	Only for limit segmentation. A vector of feature vectors to group class centers.

@param thresholds	Only for threshold segmentation. The parameter of the threshold segmentation.
<BR>Must be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership

@param type	The type of segmentation. Possible values are : max, closest, threshold, limits, fix

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

#include "../classes/Classifier.h"
#include "../classes/FCMClassifier.h"
#include "../classes/PCMClassifier.h"
#include "../classes/PFCMClassifier.h"
#include "../classes/PCM2Classifier.h"
#include "../classes/SPoCAClassifier.h"
#include "../classes/SPoCA2Classifier.h"
#include "../classes/HistogramFCMClassifier.h"
#include "../classes/HistogramPCM2Classifier.h"
#include "../classes/HistogramPCMClassifier.h"

#include "../classes/FeatureVector.h"
#include "../classes/SegmentationStats.h"
#include "../classes/FitsFile.h"


using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program does classification and segmentation.";
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
	
	args("segmentation") = Classifier::segmentationParameters();
	args("classification") = Classifier::classificationParameters();
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["type"] = ArgParser::Parameter("SPoCA2", 'T', "The type of classifier to use for the classification.\nPossible values: FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2, HFCM(Histogram FCM), HPFCM(Histogram PFCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)");
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images.\nPossible values: EIT, EUVI, AIA, SWAP");
	args["imagePreprocessing"] = ArgParser::Parameter("ALC", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images when running multi channel classification.");
	args["centersFile"] = ArgParser::Parameter("", 'c', "The name of the file containing the centers. If it it not provided the centers will be initialized randomly.");
	args["numberPreviousCenters"] = ArgParser::Parameter(0, 'n', "The number of previous centers to take into account for the median computation of final centers.");
	args["map"] = ArgParser::Parameter(true, 'M', "Set to false if you don't want to write the segmentation map.");
	args["stats"] = ArgParser::Parameter(false, 's', "Set to compute stats about the generated maps.");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=0.95", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this to true if you want results maps to be uncompressed.");
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
	
	// We setup the output directory
	string outputDirectory;
	string outputFile = args["output"];
	if (isDir(outputFile))
	{
		outputDirectory = outputFile;
	}
	else
	{
		outputDirectory = getPath(outputFile);
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	// We read and preprocess the sun images
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	vector<EUVImage*> images;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		EUVImage* image = getImageFromFile(args["imageType"], imagesFilenames[p]);
		image->preprocessing(args["imagePreprocessing"]);
		
		#if defined DEBUG
			image->getHeader().set("IPREPROC", args["imagePreprocessing"], "Image Preprocessing");
			image->writeFits(outputDirectory + "/" + stripPath(stripSuffix(imagesFilenames[p])) + ".preprocessed.fits");
		#endif
		images.push_back(image);
	}
	
	// We verify the images are aligned and we register them
	for(unsigned p = 1; p < images.size(); ++p)
	{
		string dissimilarity = checkSimilar(images[0], images[p]);
		if(! dissimilarity.empty())
		{
			if(args["registerImages"])
			{
				#if defined VERBOSE
				cout<<"Image "<<imagesFilenames[p]<<" will be registered to image "<<imagesFilenames[0]<<endl;
				#endif
				images[p]->align(images[0]);
				#if defined DEBUG
				images[p]->writeFits(outputDirectory + "/" + stripPath(stripSuffix(imagesFilenames[p])) + ".registered.fits");
				#endif
			}
			else
			{
				#if defined EXTRA_SAFE
				cerr<<"Error: image "<<imagesFilenames[p]<<" and "<<imagesFilenames[0]<<" are not similar: "<<dissimilarity<<endl;
				return EXIT_FAILURE;
				#else
				cerr<<"Warning: image "<<imagesFilenames[p]<<" and "<<imagesFilenames[0]<<" are not similar: "<<dissimilarity<<endl;
				#endif
			}
		}
	}
	
	// We setup the filename prefix
	if (isDir(outputFile))
	{
		// We set the name of the output files prefix to the outputDirectory + the classification type + image channel and date_obs
		filenamePrefix = makePath(outputDirectory, args["type"]);
		for(unsigned p = 0; p < images.size(); ++p)
			filenamePrefix += "." + images[p]->Channel() + "." + toString(images[p]->ObservationTime());
		filenamePrefix += ".";
		outputFile = filenamePrefix + "SegmentedMap.fits";
	}
	else
	{
		filenamePrefix = stripSuffix(outputFile);
	}
	
	// We initialise the Classifier
	Classifier* F;
	bool classifierIsPossibilistic = false;
	
	if (args["type"] == "FCM")
	{
		F = new FCMClassifier(args("classification"));
	}
	else if (args["type"] == "PCM")
	{
		F = new PCMClassifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "PFCM")
	{
		F = new PFCMClassifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "PCM2")
	{
		F = new PCM2Classifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "SPoCA")
	{
		F = new SPoCAClassifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "SPoCA2")
	{
		F = new SPoCA2Classifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "HFCM")
	{
		F = new HistogramFCMClassifier(args("classification"));
	}
	else if (args["type"] == "HPCM")
	{
		F = new HistogramPCMClassifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else if (args["type"] == "HPCM2")
	{
		F = new HistogramPCM2Classifier(args("classification"));
		classifierIsPossibilistic = true;
	}
	else 
	{
		cerr<<"Error : "<<args["type"]<<" is not a known classifier!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We read the channels and the initial class centers from the centers file
	vector<RealFeature> B;
	vector<string> channels;
	int numberClasses = args("classification")["numberClasses"];
	if(args["centersFile"].is_set() && isFile(args["centersFile"]) && !emptyFile(args["centersFile"]))
	{
		readCentersFromFile(args["centersFile"], channels, B);
		if(reorderImages(images, channels))
		{
			// We initialise the classifier with the centers read from the file
			F->initB(channels, B);
			// We add the images to the classifier
			F->addImages(images);
		}
		else
		{
			cerr<<"Error : The images channels do not correspond to centers channels."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	else if(numberClasses > 0)
	{
		// We add the images to the classifier
		F->addImages(images);
		
		// We initialise the centers randomly
		F->randomInitB(numberClasses);
		
		channels = F->getChannels();
	}
	else
	{
		cerr<<"Error : You must either specify the number classes or a non empty class centers file!"<<endl;
		return EXIT_FAILURE;
	}
	
	// If the classifier is probabilistic we need to do a FCM to init the etas
	if(classifierIsPossibilistic)
	{
		dynamic_cast<PCMClassifier*>(F)->FCMinit();
		#if defined VERBOSE
		cout<<"FCMinit found centers "<<F->getB()<<endl;
		#endif
	}
	
	// We do the classification
	F->classification();
	
	// We write the found centers to file
	F->sortB();
	
	#if defined DEBUG || defined WRITE_CENTERS_FILE
	if(classifierIsPossibilistic)
		writeCentersEtasToFile(filenamePrefix + "centers.txt", F->getChannels(), F->getB(), dynamic_cast<PCMClassifier*>(F)->getEta());
	else
		writeCentersToFile(filenamePrefix + "centers.txt", F->getChannels(), F->getB());
	#endif
	
	// If we need to take into account the previous centers found, we adapt the centers found by the classification
	int numberPreviousCenters = args["numberPreviousCenters"];
	if(args["centersFile"].is_set())
	{
		if(classifierIsPossibilistic)
		{
			deque<vector<RealFeature> > Bs;
			deque<vector<Real> > Etas;
			if(isFile(args["centersFile"]) && !emptyFile(args["centersFile"]))
				readCentersEtasFromFile(args["centersFile"], channels, Bs, Etas, numberPreviousCenters);
			Bs.push_front(F->getB());
			Etas.push_front(dynamic_cast<PCMClassifier*>(F)->getEta());
			dynamic_cast<PCMClassifier*>(F)->initBEta(channels, median(Bs), median(Etas));
			writeCentersEtasToFile(args["centersFile"], channels, Bs, Etas, numberPreviousCenters);
		}
		else
		{
			deque<vector<RealFeature> > Bs;
			if(isFile(args["centersFile"]) && !emptyFile(args["centersFile"]))
				readCentersFromFile(args["centersFile"], channels, Bs, numberPreviousCenters);
			Bs.push_front(F->getB());
			F->initB(channels, median(Bs));
			writeCentersToFile(args["centersFile"], channels, Bs, numberPreviousCenters);
		}
	}
	else if(numberPreviousCenters > 0)
	{
		cerr<<"Error : To use previous centers you must specify a centers file!"<<endl;
		return EXIT_FAILURE;
	}
	
	if(args["map"])
	{
		// We always terminate by an attribution
		// it sorts the class centers
		// it is needed when the Quotient Factor was bad
		// or if the classifier is histogram
		F->attribution();
		
		// We declare the segmented map with the WCS of the first image, and get the segmentation map
		ColorMap* segmentedMap = new ColorMap(images[0]->getWCS());
		F->getSegmentedMap(args("segmentation"), segmentedMap);
		
		//We add information about the classification to the header of the segmented map
		Header& header = segmentedMap->getHeader();
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			header.set(string("IMAGE")+toString(p+1,3), stripPath(imagesFilenames[p]));
		}
		header.set("CPREPROC", args["imagePreprocessing"], "Classification Image preprocessing");
		header.set("CLASTYPE", args["type"], "Classifier Type");
		if(args["stats"])
			header.set("SPREPROC", args["statsPreprocessing"], "Segmentation stats Image preprocessing");
		
		// We write the segmentedMap to a fits file
		FitsFile file(outputFile, FitsFile::overwrite);
		segmentedMap->writeFits(file, args["uncompressed"] ? 0 : FitsFile::compress, "SegmentedMap");
		
		if(args["stats"])
		{
			for (unsigned p = 0; p < imagesFilenames.size(); ++p)
			{
				EUVImage* image = getImageFromFile(args["imageType"], imagesFilenames[p]);
				image->preprocessing(args["statsPreprocessing"]);
				if(args["registerImages"])
				{
					image->align(segmentedMap);
				}
				
				// We get the RegionStats
				vector<SegmentationStats*> segmentation_stats = getSegmentationStats(segmentedMap, image);
				
				// We write the RegionStats into the fits
				file.writeTable(image->Channel()+"_SegmentationStats");
				writeRegions(file, segmentation_stats);
				for (unsigned r = 0; r < segmentation_stats.size(); ++r)
				{
					delete segmentation_stats[r];
				}
			}
		}
		delete segmentedMap;
	}
	
	// We cleanup
	delete F;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
	
	return EXIT_SUCCESS;
}
