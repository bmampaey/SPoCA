//! Program that does attribution and segmentation of EUV sun images
/*!
@page attribution attribution.x

 This program takes a tuple of EUV sun images in fits format, does the requested attribution and segmentation.
 
 A tuple of images is a list of images that have different channels / wavelength but are similar.
 
 It outputs the maps and statistics of the Active %Region (AR) and Coronal Holes (CH) 
 
 It can also output the map and statistics about the 3 classes AR, CH and Quiet Sun (QS) in general.
 
 @section usage Usage
 
 <tt> attribution.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> attribution.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one fits file per channel.
 The order of the fits files is important, as the first one will be used for the statistics of the regions.
 
 
@param classifierType	The type of classifier to use for the attribution.
<BR> Possible values are : 
 - FCM
 - PFCM
 - PCM
 - PCM2
 - SPOCA
 - SPOCA2
 - HFCM(Histogram FCM)
 - HPFCM(Histogram PFCM)
 - HPCM(Histogram PCM)
 - HPCM2(Histogram PCM2)

@param maxNumberIteration	The maximal number of iteration for the attribution.
 
@param precision	The precision to be reached to stop the attribution.

@param fuzzifier	The fuzzifier (m).

@param numberClasses	The number of classes to classify the sun images into.

@param centersFile	The name of the file containing the centers.
 If it it not provided the centers will be initialized randomly.

@param numberPreviousCenters	The number of previous saved centers to take into account for the final attribution.
<BR> If you are running in continuous mode, this option allow to store n centers in the centers files. It is the median value of those n centers + the last found one that will to be used for the last attribution.

@param imageType	The type of the images.
<BR>Possible values are : 
 - EIT
 - EUVI
 - AIA
 - SWAP

@param preprocessingSteps	The steps of preprocessing to apply to the sun images.
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

@param radiusratio	The ratio of the radius of the sun that will be processed.

@param maps	The kind of maps to generate.
<BR>Possible values :
 - A (Active %Region)
 - C (Coronal Hole)
 - S (Segmented)

@param intensitiesStatsRadiusRatio	The ratio of the radius of the sun that will be used for the region stats.

@param intensitiesStatsPreprocessing	The steps of preprocessing to apply to the sun images (see preprocessingSteps for possible values).

@param neighborhoodRadius	The neighborhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3. <BR>Only for spatial classifiers like SPoCA.


@param histogramFile	The name of a file containing an histogram.

@param binSize	The size of the bins of the histogram.
<BR>N.B. Be carreful that the histogram is built after the preprocessing.

@param segmentation	The segmentation type.
<BR>Possible values :
 - max (Maximum of Uij)
 - closest (Closest center)
 - threshold (Threshold on Uij)
 - limits (Merge on centers value limits)
 - fix (Merge on fix CH QS AR)

@param maxLimitsFile	The name of the file containing the max limits. Only for limit segmentation.
 

@param ch	The classes of the Coronal Hole. <BR>Only for fix segmentation.

@param qs	The classes of the Quiet Sun. <BR>Only for fix segmentation.

@param ar	The classes of the Active %Region. <BR>Only for fix segmentation.

@param tr	The parameter of the threshold segmentation.
Must be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership <BR>Only for threshold segmentation.

@param uncompressedMaps	Set this flag if you want results maps to be uncompressed.

@param chaincodeMaxPoints The maximal number of points in a chaincode.

@param chaincodeMaxDeviation The maximal deviation of the chaincode curve between 2 points, in arcsec.


@param outputDirectory	The name for the output directory.

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

#include "../classes/FeatureVector.h"
#include "../classes/SegmentationStats.h"
#include "../classes/FitsFile.h"


using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program does attribution and segmentation.";
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
	
	args["type"] = ArgParser::Parameter("SPoCA2", 'T', "The type of classifier to use for the attribution. Possible values are : FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2");
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images. Possible values are : EIT, EUVI, AIA, SWAP");
	args["imagePreprocessing"] = ArgParser::Parameter("ALC", 'P', "The steps of preprocessing to apply to the sun images before attribution. Can be any combination of the following: NAR=zz.z (Nullify pixels above zz.z*radius); ALC (Annulus Limb Correction); DivMedian (Division by the median); TakeSqrt (Take the square root); TakeLog (Take the log); DivMode (Division by the mode); DivExpTime (Division by the Exposure Time); ThrMin=zz.z (Threshold intensities to minimum zz.z); ThrMax=zz.z (Threshold intensities to maximum zz.z); ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile); ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile); Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images when running multi channel attribution.");
	args["centersFile"] = ArgParser::Parameter('c', "The name of the file containing the centers. If it it not provided the centers will be initialized randomly.");
	args["computeEta"] = ArgParser::Parameter(false, 'e', "If the enters file do not contain the values for Eta or if you want to force Eta to be recomputed (slow!).");
	args["stats"] = ArgParser::Parameter(false, 's', "Set to compute stats about the generated maps.");
	args["statsPreprocessing"] = ArgParser::Parameter("NAR=0.95", 'p', "The steps of preprocessing to apply to the sun images before computing stats. Can be any combination of the following: NAR=zz.z (Nullify pixels above zz.z*radius); ALC (Annulus Limb Correction); DivMedian (Division by the median); TakeSqrt (Take the square root); TakeLog (Take the log); DivMode (Division by the mode); DivExpTime (Division by the Exposure Time); ThrMin=zz.z (Threshold intensities to minimum zz.z); ThrMax=zz.z (Threshold intensities to maximum zz.z); ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile); ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile); Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
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
	else 
	{
		cerr<<"Error : "<<args["type"]<<" is not an accepted classifier!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We read the channels and the initial class centers from the centers file
	vector<RealFeature> B;
	vector<Real> Eta;
	vector<string> channels;
	if(args["centersFile"].is_set() && isFile(args["centersFile"]) && !emptyFile(args["centersFile"]))
	{
		if(classifierIsPossibilistic)
		{
			if(args["computeEta"])
			{
				if(! readCentersFromFile(args["centersFile"], channels, B))
				{
					cerr<<"Error: could not read centers from file!"<<endl;
					return EXIT_FAILURE;
				}
				else if(!reorderImages(images, channels))
				{
					cerr<<"Error : The images channels do not correspond to centers channels."<<endl;
					return EXIT_FAILURE;
				}
				else
				{
					// We initialise the classifier with the centers read from the file
					F->initB(channels, B);
					// We add the images to the classifier
					F->addImages(images);
					// We compute the Eta
					dynamic_cast<PCMClassifier*>(F)->FCMinit();
				}
			}
			else
			{
				if(! readCentersEtasFromFile(args["centersFile"], channels, B, Eta) || Eta.empty())
				{
						cerr<<"Error: could not read centers and/or eta from file!"<<endl;
						return EXIT_FAILURE;
				}
				else if(!reorderImages(images, channels))
				{
					cerr<<"Error : The images channels do not correspond to centers channels."<<endl;
					return EXIT_FAILURE;
				}
				else
				{
					// We initialise the classifier with the centers and eta read from the file
					dynamic_cast<PCMClassifier*>(F)->initBEta(channels, B, Eta);
					// We add the images to the classifier
					F->addImages(images);
				}
			}
		}
		else
		{
			if(! readCentersFromFile(args["centersFile"], channels, B))
			{
				cerr<<"Error: could not read centers from file!"<<endl;
				return EXIT_FAILURE;
			}
			else if(!reorderImages(images, channels))
			{
				cerr<<"Error : The images channels do not correspond to centers channels."<<endl;
				return EXIT_FAILURE;
			}
			else
			{
				// We initialise the classifier with the centers read from the file
				F->initB(channels, B);
				// We add the images to the classifier
				F->addImages(images);
			}
		}
	}
	else
	{
		cerr<<"Error : For attribution you must provide the centers."<<endl;
		return EXIT_FAILURE;
	}
	
	// We do the attribution
	F->attribution();
	
	// We declare the segmented map with the WCS of the first image, and get the segmentation map
	ColorMap* segmentedMap = new ColorMap(images[0]->getWCS());
	F->getSegmentedMap(args("segmentation"), segmentedMap);
	
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
	
	// We cleanup
	delete segmentedMap;
	delete F;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
	
	return EXIT_SUCCESS;
}
