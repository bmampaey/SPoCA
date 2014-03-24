//! This Program does attribution and computes SOLID stats (aka. ring analysis).
/*!
@page get_SOLID_stats get_SOLID_stats.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/get_SOLID_stats.x [-option optionvalue ...]  fitsFile fitsFile </tt>

@param fitsFile	Path to a fits file

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param centersFile	The name of the file containing the centers. If it it not provided the centers will be initialized randomly.

@param computeEta	If the enters file do not contain the values for Eta or if you want to force Eta to be recomputed (slow!).

@param fuzzyStats	Set this flag if you want fuzzy ring stats.

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

@param output	The name for the output file or of a directory.

@param registerImages	Set to register/align the images when running multi channel attribution.

@param type	The type of classifier to use for the attribution.
<BR>Possible values: FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2

@param uncompressed	Set this flag if you want results maps to be uncompressed.

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

#include "../classes/FitsFile.h"


using std::string; using std::cout; using std::cerr; using std::endl;
using std::vector; using std::deque;
using std::ofstream; using std::ostringstream;
using std::ios; using std::ios_base;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

// The rings
float rings[] = {0.07, 0.16, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95, 1.05, 1.15, 1.25, 1.35};
unsigned number_rings = sizeof rings / sizeof rings[0];

vector<float> get_fuzzy_ring_stats(const EUVImage* fuzzyMap)
{
	// We initialise the vector of results
	vector<float> stats(number_rings + 2, 0);
	
	RealPixLoc sun_center = fuzzyMap->SunCenter();
	Real sun_radius = fuzzyMap->SunRadius();
	
	//We compute for each pixel it's ring position
	RealPixLoc location;
	for(location.y = 0;  location.y < fuzzyMap->Yaxes(); ++location.y)
	{
		for(location.x = 0; location.x < fuzzyMap->Xaxes(); ++location.x)
		{
			Real d = distance(location, sun_center) / sun_radius;
			if (d < 1)
				stats[0] += fuzzyMap->pixel(location.x, location.y);
			
			bool stop = false;
			for (unsigned r = 0; r < number_rings && !stop; ++r)
			{
				if (d < rings[r])
				{
					stats[r + 1] += fuzzyMap->pixel(location.x, location.y);
					stop = true;
				}
			}
			if(! stop)
			{
				stats[number_rings + 1] += fuzzyMap->pixel(location.x, location.y);
			}
		}
	}
	return stats;
}

vector< vector<float> > get_segmented_ring_stats(const ColorMap* segmentedMap, const unsigned number_classes)
{
	// We initialise the vector of results
	vector< vector<float> > stats(number_classes + 1);
	for (unsigned i = 0; i < number_classes + 1; ++i)
	{
		stats[i].resize(number_rings + 2, 0);
	}
	
	
	RealPixLoc sun_center = segmentedMap->SunCenter();
	Real sun_radius = segmentedMap->SunRadius();
	
	//We compute for each pixel it's ring position
	RealPixLoc location;
	for(location.y = 0;  location.y < segmentedMap->Yaxes(); ++location.y)
	{
		for(location.x = 0; location.x < segmentedMap->Xaxes(); ++location.x)
		{
			Real d = distance(location, sun_center) / sun_radius;
			if (d < 1)
				++stats[segmentedMap->pixel(location.x, location.y)][0];
			bool stop = false;
			for (unsigned r = 0; r < number_rings && !stop; ++r)
			{
				if (d < rings[r])
				{
					++stats[segmentedMap->pixel(location.x, location.y)][r + 1];
					stop = true;
				}
			}
			if(! stop)
			{
				++stats[segmentedMap->pixel(location.x, location.y)][number_rings + 1];
			}
		}
	}
	return stats;
}

void write_ring_stats(const string& filename, const vector< vector<float> >& stats, const string& date_obs)
{
	string header = "time";
	string values = date_obs;
	for (unsigned r = 0; r < stats[0].size(); ++r)
	{
		for (unsigned i = 0; i < stats.size(); ++i)
		{
			header += ", ('";
			if (r == 0)
				header += "disc";
			else if (r == number_rings + 1)
				header += "other";
			else
				header += "ring" + toString(r);
			header += "'; '";
			if (i == 0)
				header += "noclass";
			else
				header += "class" + toString(i);
			header += "')";
			if (r < stats[i].size())
				values += ", " + toString(stats[i][r]);
			else
				values += ", 0";
		}
	}
	// We write the header and the values to the output file
	ofstream outputFile(filename.c_str(), ios_base::trunc);
	outputFile<<setiosflags(ios::fixed);
	outputFile<<header<<"\n"<<values<<"\n";
	outputFile.close();
}

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program does attribution and computes SOLID stats (aka. ring analysis).";
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
	
	args["type"] = ArgParser::Parameter("SPoCA2", 'T', "The type of classifier to use for the attribution.\nPossible values: FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2");
	args["imageType"] = ArgParser::Parameter("Unknown", 'I', "The type of the images.\nPossible values: EIT, EUVI, AIA, SWAP");
	args["imagePreprocessing"] = ArgParser::Parameter("ALC", 'P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images when running multi channel attribution.");
	args["centersFile"] = ArgParser::Parameter('c', "The name of the file containing the centers. If it it not provided the centers will be initialized randomly.");
	args["computeEta"] = ArgParser::Parameter(false, 'e', "If the enters file do not contain the values for Eta or if you want to force Eta to be recomputed (slow!).");
	args["map"] = ArgParser::Parameter(true, 'M', "Set to false if you don't want to write the segmentation map.");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file or of a directory.");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
	args["fuzzyStats"] = ArgParser::Parameter(false, 'F', "Set this flag if you want fuzzy ring stats.");
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file", NUMBERCHANNELS, NUMBERCHANNELS);
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch(const std::invalid_argument& error)
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
		outputFile = filenamePrefix + "csv";
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
	
	// We compute the stats
	vector< vector<float> > stats;
	unsigned numberClasses = B.size();
	if(args["fuzzyStats"])
	{
		// We declare the fuzzyMap with the WCS of the first image
		EUVImage* fuzzyMap = new EUVImage(images[0]->getWCS());
		
		// We add information about the attribution to the header
		Header& header = fuzzyMap->getHeader();
		F->fillHeader(header);
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			header.set(string("IMAGE")+toString(p+1,3), stripPath(imagesFilenames[p]));
		}
		header.set("CPREPROC", args["imagePreprocessing"], "Classification Image preprocessing");
		header.set("CLASTYPE", args["type"], "Classifier Type");
		
		stats.push_back(vector<float>(number_rings + 2, 0));
		
		// We get the fuzzy map for each class
		for (unsigned i = 0; i < numberClasses; ++i)
		{
			F->normalizedFuzzyMap(i, fuzzyMap);
			
			// We set the class number
			header.set("CLASSNBR", i + 1, "Number of the class of the fuzzy map");
			
			// We compute the ring analysis for that class
			stats.push_back(get_fuzzy_ring_stats(fuzzyMap));
			
			// We write down the maps
			if(args["map"])
			{
				FitsFile file(filenamePrefix + "FuzzyMap." + toString(i+1) + ".fits", FitsFile::overwrite);
				fuzzyMap->writeFits(file, args["uncompressed"] ? 0 : FitsFile::compress, "FuzzyMap");
			}
		}
		
		delete fuzzyMap;
	}
	else
	{
		// We declare the segmented map with the WCS of the first image
		ColorMap* segmentedMap = new ColorMap(images[0]->getWCS());
		
		// We get the segmentation map
		F->getSegmentedMap(args("segmentation"), segmentedMap);
		
		//We add information about the classification to the header of the segmented map
		Header& header = segmentedMap->getHeader();
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			header.set(string("IMAGE")+toString(p+1,3), stripPath(imagesFilenames[p]));
		}
		header.set("CPREPROC", args["imagePreprocessing"], "Classification Image preprocessing");
		header.set("CLASTYPE", args["type"], "Classifier Type");
		
		// We compute the ring analysis
		stats = get_segmented_ring_stats(segmentedMap, numberClasses);
		
		// We write down the map
		if(args["map"])
		{
			FitsFile file(filenamePrefix + "SegmentedMap.fits", FitsFile::overwrite);
			segmentedMap->writeFits(file, args["uncompressed"] ? 0 : FitsFile::compress, "SegmentedMap");
		}
		
		delete segmentedMap;
	}
	
	// We output the ring stats to file
	write_ring_stats(outputFile, stats, images[0]->ObservationDate());
	
	// We cleanup
	delete F;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
	
	return EXIT_SUCCESS;
}
