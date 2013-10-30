//! Program that does attribution (fix classification) and extract the ring stats
/*!
@page get_ring_stats get_ring_stats.x

 This program takes a tuple of EUV sun images in fits format, does the requested attribution and extract the ring statistics.
 
 A tuple of images is a list of images that have different channels / wavelengths but are similar.
 
 It outputs the statistics of the ring nalysis for SOLID
 
 @section usage Usage
 
 <tt> get_ring_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_ring_stats.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one fits file per channel.
 The order of the fits files is important, as the first one will be used for the statistics of the regions.
 
 
@param classifierType	The type of classifier to use for the classification.
<BR> Possible values are : 
 - FCM
 - PCM
 - PCM2
 - SPOCA
 - SPOCA2

@param maxNumberIteration	The maximal number of iteration for the classification.
 
@param precision	The precision to be reached to stop the classification.

@param fuzzifier	The fuzzifier (m).

@param numberClasses	The number of classes to classify the sun images into.

@param centersFile	The name of the file containing the centers.
 It is mandatory for attribution

@param etaFile	The name of the file containing eta
<BR>N.B. Be carefull that the order of the eta must be the same than the order of the centers in the centersFile!
<BR>If it it not provided the eta will be computed with a FCM (then it is not a real attribution).

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


@param neighboorhoodRadius	The neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3. <BR>Only for spatial classifiers like SPoCA.

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
#include "../classes/ArgumentHelper.h"

#include "../classes/EUVImage.h"
#include "../classes/ColorMap.h"

#include "../classes/Classifier.h"
#include "../classes/FCMClassifier.h"
#include "../classes/PCMClassifier.h"
#include "../classes/PCM2Classifier.h"
#include "../classes/SPoCAClassifier.h"
#include "../classes/SPoCA2Classifier.h"


#include "../classes/FeatureVector.h"
#include "../classes/RegionStats.h"
#include "../classes/SegmentationStats.h"
#include "../classes/ActiveRegion.h"
#include "../classes/CoronalHole.h"
#include "../classes/FitsFile.h"

using std::string; using std::cout; using std::cerr; using std::endl;
using std::vector; 
using std::ofstream; using std::ostringstream;
using std::ios; using std::ios_base;
using namespace dsr;

string filenamePrefix;

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
				header += "ring" + itos(r);
			header += "'; '";
			if (i == 0)
				header += "noclass";
			else
				header += "class" + itos(i);
			header += "')";
			if (r < stats[i].size())
				values += ", " + dtos(stats[i][r]);
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

	cout<<setiosflags(ios::fixed);

	// Program version
	string version = "2.0";

	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

	// The number of classes to classify into
	unsigned numberClasses = 0;
	
	// Option to pass a class center file
	string centersFileName;
	
	// Options for the classification
	string classifierType = "FCM";
	unsigned maxNumberIteration = 100;
	double precision = 0.0015;
	double fuzzifier = 2;
		
	// Option for the possibilisctic Classifiers
	string etaFileName;
		
	// Option for the Spacial Classifiers (SPoCA)
	unsigned neighboorhoodRadius = 1;
	
	// Options for the segmentation
	string segmentation = "max";
	
	// Options for the limit segmentation
	string maxLimitsFileName;
	
	// Options for the fix segmentation
	string coronalHole;
	string quietSun;
	string activeRegion;
	
	// Options for the threshold segmentation
	string threshold;

	// option for the output directory
	string outputDirectory = ".";
	
	// Options for the desired maps
	bool getMaps = false;
	bool uncompressedMaps = false;
	bool getFuzzyStats = false;
	
	// We parse the arguments

	string programDescription = "This Programm does attribution (or fix classification) and segmentation.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('T',"classifierType", "string", "\n\tThe type of classifier to use for the classification.\n\tPossible values are : FCM, PCM, PCM2, SPOCA, SPOCA2\n\t", classifierType);
	arguments.new_named_unsigned_int('i', "maxNumberIteration", "positive integer", "\n\tThe maximal number of iteration for the classification.\n\t", maxNumberIteration);
	arguments.new_named_double('p',"precision", "positive real", "\n\tThe precision to be reached to stop the classification.\n\t",precision);
	arguments.new_named_double('f',"fuzzifier", "positive real", "\n\tThe fuzzifier (m).\n\t",fuzzifier);
	arguments.new_named_unsigned_int('C', "numberClasses", "positive integer", "\n\tThe number of classes to classify the sun images into.\n\t", numberClasses);
	arguments.new_named_string('B',"centersFile","file name", "\n\tThe name of the file containing the centers.\n\tMandatory for attribution!\n\t", centersFileName);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('E',"etaFile","file name", "\n\tThe name of the file containing eta.\n\tBe carefull that the order of the eta must be the same than the order of the centers in the centersFile!\n\tIf it it not provided the eta will be computed with a FCM (then it is not a real attribution).\n\t", etaFileName);
	arguments.new_named_unsigned_int('N', "neighboorhoodRadius", "positive integer", "\n\tOnly for spatial classifiers like SPoCA.\n\tThe neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3.\n\t", neighboorhoodRadius);
	arguments.new_named_string('S', "segmentation", "string", "\n\tThe segmentation type.\n\tPossible values :\n\t\tmax (Maximum of Uij)\n\t\tclosest (Closest center)\n\t\tthreshold (Threshold on Uij)\n\t\tlimits (Merge on centers value limits)\n\t\tfix (Merge on fix CH QS AR)\n\t", segmentation);
	arguments.new_named_string('L',"maxLimitsFile","file", "\n\tOnly for limit segmentation.\n\tThe name of the file containing the max limits.\n\t", maxLimitsFileName);
	arguments.new_named_string('c',"ch","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Coronal Hole.\n\t", coronalHole);
	arguments.new_named_string('q',"qs","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Quiet Sun.\n\t", quietSun);
	arguments.new_named_string('a',"ar","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Active Region.\n\t", activeRegion);
	arguments.new_named_string('t',"tr","coma separated list of positive integer (no spaces)", "\n\tOnly for threshold segmentation.\n\tThe parameter of the threshold segmentation.\n\tMust be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership\n\t", threshold);
	arguments.new_flag('F', "getFuzzyStats", "\n\tSet this flag if you want fuzzy ring stats.\n\t", getFuzzyStats);
	arguments.new_flag('m', "getMaps", "\n\tSet this flag if you want the segmentation maps to be written.\n\t", getMaps);
	arguments.new_flag('u', "uncompressedMaps", "\n\tSet this flag if you want results maps to be uncompressed.\n\t", uncompressedMaps);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version.c_str());
	arguments.process(argc, argv);

	
	// General variables
	vector<RealFeature> B;
	vector<string> channels;
	Classifier* F;
	vector<Real> eta;

	bool classifierIsPossibilistic = false;

	// We process the arguments

	// We assert that the number of sun images provided is correct
	if(imagesFilenames.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file given as parameter, "<<NUMBERCHANNELS<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}


	// We check if the outputDirectory is a directory 
	if (! isDir(outputDirectory))
	{
		filenamePrefix = outputDirectory+".";
		outputDirectory = getPath(outputDirectory);
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	// We read the channels and the initial centers from the centers file
	if(readCentersFromFile(B, channels, centersFileName))
	{
		if(B.size() == 0)
		{
			cerr<<"Error : For attribution you must provide the centers."<<endl;
		
		}
		if(B.size() != numberClasses)
		{
			cerr<<"Error : The number of classes is different than the number of centers read in the center file."<<endl;
			numberClasses = B.size();
			cerr<<"The number of classes will be set to "<<numberClasses<<endl;
			
		}
	}
	
	// We save the centers
	writeCentersToFile(B, channels, filenamePrefix + "centers.txt");
	
	
	// We declare the type of Classifier we want
	if (classifierType == "FCM")
	{
		F = new FCMClassifier(fuzzifier);
	}
	else if (classifierType == "PCM")
	{
		F = new PCMClassifier(fuzzifier);
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "PCM2")
	{
		F = new PCM2Classifier(fuzzifier);
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "SPoCA")
	{
		F = new SPoCAClassifier(neighboorhoodRadius, fuzzifier);
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "SPoCA2")
	{
		F = new SPoCA2Classifier(neighboorhoodRadius, fuzzifier);
		classifierIsPossibilistic = true;
	}
	else 
	{
		cerr<<"Error : "<<classifierType<<" is not a known classifier!"<<endl;
		return EXIT_FAILURE;
	}
	
		
	// We read and preprocess the sun images
	vector<EUVImage*> images = getImagesFromFiles(imageType, imagesFilenames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
		#if DEBUG >= 2
		images[p]->writeFits(outputDirectory + "/" + stripPath(stripSuffix(imagesFilenames[p])) + ".preprocessed.fits");
		#endif
		
		// We check if the images are similars
		string dissimilarity = checkSimilar(images[0], images[p]);
		if(! dissimilarity.empty())
		{
			cerr<<"Warning: image "<<imagesFilenames[p]<<" and "<<imagesFilenames[0]<<" are not similar: "<<dissimilarity<<endl;
		}
	}

	// We set the name of the output files prefix
	// to the outputDirectory + the date_obs of the first image in the form YYYYMMDD_HHMMSS
	if(filenamePrefix.empty())
		filenamePrefix = outputDirectory + "/" + time2string(images[0]->ObservationTime()) + ".";

	// We add the images to the classifier
	F->addImages(images);
		
	// We save the WCS of the first image
	WCS wcs = images[0]->getWCS();
	
	// We delete all images to gain memory space
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
		
		
	// We initialize the Classifier with the centers from the centers file
	F->initB(B, channels);

	if(classifierIsPossibilistic)
	{
		// We read the eta from the eta file
		if(readEtaFromFile(eta, etaFileName))
		{
			if(B.size() != eta.size())
			{
				cerr<<"Error : The number of centers read in the center file is different than the number of eta read in the eta file."<<endl;
				return EXIT_FAILURE;
			}
		}
		
		if(eta.size() != 0 )
		{
			dynamic_cast<PCMClassifier*>(F)->initEta(eta);
		}
		else
		{	// We probably shouldn't come here
			dynamic_cast<PCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
			F->initB(B, channels);
		}
		
		// We save the eta
		dynamic_cast<PCMClassifier*>(F)->saveEta(filenamePrefix + "eta.txt");
	}	
	
	#if DEBUG >= 3
	cout<<"The centers have been initialized to B :"<<F->getB()<<endl;
	if(classifierIsPossibilistic)
	{
		cout<<"The eta have been initialized to :"<<dynamic_cast<PCMClassifier*>(F)->getEta()<<endl;
	}
	#endif

	// We have all the information we need, we can do the attribution
	F->attribution();
	
	// We compute the stats and output the maps
	vector< vector<float> > stats;
	if (getFuzzyStats)
	{
		// We declare the fuzzyMap with the WCS of the first image
		EUVImage* fuzzyMap = new EUVImage(wcs);
		
		// We add information about the attribution to the header
		Header& header = fuzzyMap->getHeader();
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			header.set(string("IMAGE")+itos(p+1,3), stripPath(imagesFilenames[p]));
		}

		header.set("CVERSION", version, "SPoCA Version");
		header.set("CPREPROC", preprocessingSteps, "Preprocessing Steps");
		header.set("CRADRATI", radiusRatio, "Radius Ratio");
		header.set("CLASTYPE", classifierType, "Classifier Type");
		header.set("CNBRCLAS", numberClasses, "Number Classes");
		header.set("CPRECIS", precision, "Classifier Precision");
		header.set("CMAXITER", maxNumberIteration, "Max Number Iteration");
		header.set("CFUZFIER", fuzzifier, "Classifier Fuzzifier");
		header.set("CHANNELS", vtos(F->getChannels()), "Classification Channels");
	
		B = F->getB();
		for (unsigned i = 0; i < numberClasses; ++i)
			header.set("CLSCTR"+itos(i,2), B[i].toString(4), "Classification class center " + itos(i,2));
	
		if(classifierIsPossibilistic)
		{
			ostringstream ss;
			ss<<dynamic_cast<PCMClassifier*>(F)->getEta();
			header.set("CETA", ss.str(), "Classification eta");
		}
		
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
			if(getMaps)
			{
				FitsFile file(filenamePrefix + "FuzzyMap." + itos(i+1) + ".fits", FitsFile::overwrite);
				fuzzyMap->writeFits(file, uncompressedMaps ? 0 : FitsFile::compress, "FuzzyMap");
			}
		}
		delete fuzzyMap;
	}
	else
	{
		// We declare the segmented map with the WCS of the first image
		ColorMap* segmentedMap = new ColorMap(wcs);
		
		// We add information about the attribution to the header
		Header& header = segmentedMap->getHeader();
		for (unsigned p = 0; p < imagesFilenames.size(); ++p)
		{
			header.set(string("IMAGE")+itos(p+1,3), stripPath(imagesFilenames[p]));
		}

		header.set("CVERSION", version, "SPoCA Version");
		header.set("CPREPROC", preprocessingSteps, "Preprocessing Steps");
		header.set("CRADRATI", radiusRatio, "Radius Ratio");
		header.set("CLASTYPE", classifierType, "Classifier Type");
		header.set("CNBRCLAS", numberClasses, "Number Classes");
		header.set("CPRECIS", precision, "Classifier Precision");
		header.set("CMAXITER", maxNumberIteration, "Max Number Iteration");
		header.set("CFUZFIER", fuzzifier, "Classifier Fuzzifier");

		header.set("SEGMTYPE", segmentation, "Segmentation type");
		if(! activeRegion.empty())
			header.set("SFIXAR", activeRegion);
		if(! quietSun.empty())
			header.set("SFIXQS", quietSun);
		if(! coronalHole.empty())
			header.set("SFIXCH", coronalHole);
		if(! threshold.empty())
			header.set("STRSHLD", threshold);
		header.set("CHANNELS", vtos(F->getChannels()), "Classification Channels");
	
		B = F->getB();
		for (unsigned i = 0; i < numberClasses; ++i)
			header.set("CLSCTR"+itos(i,2), B[i].toString(4), "Classification class center " + itos(i,2));
	
		if(classifierIsPossibilistic)
		{
			ostringstream ss;
			ss<<dynamic_cast<PCMClassifier*>(F)->getEta();
			header.set("CETA", ss.str(), "Classification eta");
		}
	
		// We do the segmentation
		if (segmentation == "max")
		{
			F->segmentedMap_maxUij(segmentedMap);
		}
		else if (segmentation == "closest")
		{
			F->segmentedMap_closestCenter(segmentedMap);
		}
		else if (segmentation == "threshold")
		{
			char delimitor;
			unsigned class_number;
			Real lowerIntensity_minMembership, higherIntensity_minMembership;
			threshold>>class_number>>delimitor>>lowerIntensity_minMembership>>delimitor>>higherIntensity_minMembership;
			F->segmentedMap_classThreshold(class_number, lowerIntensity_minMembership, higherIntensity_minMembership, segmentedMap);
		}
		else if (segmentation == "limits")
		{
			vector<RealFeature> maxLimits;
			if(maxLimitsFileName.empty())
			{
				cerr<<"Error : For limits segmentation the maxLimitsFile is mandatory."<<endl;
				return EXIT_FAILURE;
			}
			else
			{
				readMaxLimitsFromFile(maxLimits, maxLimitsFileName);
			}
			F->segmentedMap_limits(maxLimits, segmentedMap);
		}
		else if (segmentation == "fix")
		{
			vector<unsigned> ch, qs, ar;
			if(!coronalHole.empty())
			{
				coronalHole>>ch;
			}
			if(!quietSun.empty())
			{
				quietSun>>qs;
			}
			if(!activeRegion.empty())
			{
				activeRegion>>ar;
			}
			F->segmentedMap_fixed(ch, qs, ar, segmentedMap);
		}
		else 
		{
			cerr<<"Error : "<<segmentation<<" is not a known segmentation!"<<endl;
			return EXIT_FAILURE;
		}
		
		// We compute the ring analysis
		stats = get_segmented_ring_stats(segmentedMap, numberClasses);
		
		// We write down the map
		if(getMaps)
		{
			FitsFile file(filenamePrefix + "SegmentedMap.fits", FitsFile::overwrite);
			segmentedMap->writeFits(file, uncompressedMaps ? 0 : FitsFile::compress, "SegmentedMap");
		}
	
		delete segmentedMap;
	}
	
	// We output the ring stats to file
	write_ring_stats(filenamePrefix + "ring_stats.csv", stats, wcs.date_obs);
	delete F;
	return EXIT_SUCCESS;
}
