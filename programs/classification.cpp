//! Program that does classification and segmentation of EUV sun images
/*!
@page classification classification.x

 This program takes a tuple of EUV sun images in fits format, does the requested classification and segmentation.
 
 A tuple of images is a list of images that have different channels / wavelength but are similar.
 
 It outputs the maps and statistics of the Active %Region (AR) and Coronal Holes (CH) 
 
 It can also output the map and statistics about the 3 classes AR, CH and Quiet Sun (QS) in general.
 
 @section usage Usage
 
 <tt> classification.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> classification.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one fits file per channel.
 The order of the fits files is important, as the first one will be used for the statistics of the regions.
 
 
@param classifierType	The type of classifier to use for the classification.
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

@param maxNumberIteration	The maximal number of iteration for the classification.
 
@param precision	The precision to be reached to stop the classification.

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
 
@param radiusratio	The ratio of the radius of the sun that will be processed.

@param maps	The kind of maps to generate.
<BR>Possible values :
 - A (Active %Region)
 - C (Coronal Hole)
 - S (Segmented)

@param regionStatsRadiusRatio	The ratio of the radius of the sun that will be used for the region stats.

@param regionStatsPreprocessing	The steps of preprocessing to apply to the sun images (see preprocessingSteps for possible values).

@param neighboorhoodRadius	The neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3. <BR>Only for spatial classifiers like SPoCA.


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
 

@param thresholdRawArea	Set this flag if you want the Active Regions and Coronal Holes to be thresholded according to their Raw area instead of their center area

@param uncompressed_results	Set this flag if you want results maps to be uncompressed.

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
#include "../classes/RegionStats.h"
#include "../classes/ActiveRegion.h"
#include "../classes/ActiveRegionStats.h"
#include "../classes/CoronalHole.h"
#include "../classes/CoronalHoleStats.h"
#include "../classes/FitsFile.h"


using namespace std;
using namespace dsr;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);

	// Program version
	const char * version = "2.0";

	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

	// The number of classes to classify into
	unsigned numberClasses = 0;
	
	//The number of previos centers to take into account into the computation of the final class centers
	unsigned numberPreviousCenters = 0;
	
	// Option to pass a class center file
	string centersFileName;

	// Options for the classification
	string classifierType = "HFCM";
	unsigned maxNumberIteration = 100;
	double precision = 0.0015;
	double fuzzifier = 2;
		
	// Option for the Spacial Classifiers (SPoCA)
	unsigned neighboorhoodRadius = 1;
	
	// Options for the histogram classifiers
	string sbinSize;
	string histogramFile;
	
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
	
	// Options for the ARmap and CHmap
	bool thresholdRawArea = false;
	double regionStatsRadiusRatio = 0.95;
	string regionStatsPreprocessing = "NAR";

	// option for the output directory
	string outputDirectory = ".";
	
	// Options for the desired outputs 
	bool uncompressed_results = false; 
	string desiredMaps;


	// We parse the arguments

	string programDescription = "This Programm does classification and segmentation.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('T',"classifierType", "string", "\n\tThe type of classifier to use for the classification.\n\tPossible values are : FCM, PFCM, PCM, PCM2, SPoCA, SPoCA2, HFCM(Histogram FCM), HPFCM(Histogram PFCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)\n\t", classifierType);
	arguments.new_named_unsigned_int('i', "maxNumberIteration", "positive integer", "\n\tThe maximal number of iteration for the classification.\n\t", maxNumberIteration);
	arguments.new_named_double('p',"precision", "positive real", "\n\tThe precision to be reached to stop the classification.\n\t",precision);
	arguments.new_named_double('f',"fuzzifier", "positive real", "\n\tThe fuzzifier (m).\n\t",fuzzifier);
	arguments.new_named_unsigned_int('C', "numberClasses", "positive integer", "\n\tThe number of classes to classify the sun images into.\n\t", numberClasses);
	arguments.new_named_string('B',"centersFile","file name", "\n\tThe name of the file containing the centers.\n\tIf it it not provided the centers will be initialized randomly.\n\t", centersFileName);
	arguments.new_named_unsigned_int('b',"numberPreviousCenters","positive integer", "\n\tThe number of previous saved centers to take into account for the final attribution.\n\t", numberPreviousCenters);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('M', "maps", "comma separated list of char (no spaces)", "\n\tThe kind of maps to generate.\n\tPossible values :\n\t\tA (Active Region)\n\t\tC (Coronal Hole)\n\t\tS (Segmented)\n\t", desiredMaps);
	arguments.new_named_double('R', "regionStatsRadiusRatio", "positive real", "\n\tThe ratio of the radius of the sun that will be used for the region stats.\n\t",regionStatsRadiusRatio);
	arguments.new_named_string('G', "regionStatsPreprocessing", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images (see preprocessingSteps for possible values).\n\t",regionStatsPreprocessing);
	arguments.new_named_unsigned_int('N', "neighboorhoodRadius", "positive integer", "\n\tOnly for spatial classifiers like SPoCA.\n\tThe neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3.\n\t", neighboorhoodRadius);
	arguments.new_named_string('H', "histogramFile","file name", "\n\tThe name of a file containing an histogram.\n\t", histogramFile);
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogram.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_string('S', "segmentation", "string", "\n\tThe segmentation type.\n\tPossible values :\n\t\tmax (Maximum of Uij)\n\t\tclosest (Closest center)\n\t\tthreshold (Threshold on Uij)\n\t\tlimits (Merge on centers value limits)\n\t\tfix (Merge on fix CH QS AR)\n\t", segmentation);
	arguments.new_named_string('L',"maxLimitsFile","file", "\n\tOnly for limit segmentation.\n\tThe name of the file containing the max limits.\n\t", maxLimitsFileName);
	arguments.new_named_string('c',"ch","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Coronal Hole.\n\t", coronalHole);
	arguments.new_named_string('q',"qs","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Quiet Sun.\n\t", quietSun);
	arguments.new_named_string('a',"ar","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Active Region.\n\t", activeRegion);
	arguments.new_named_string('t',"tr","coma separated list of positive integer (no spaces)", "\n\tOnly for threshold segmentation.\n\tThe parameter of the threshold segmentation.\n\tMust be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership\n\t", threshold);	
	arguments.new_flag('w', "thresholdRawArea", "\n\tSet this flag if you want the Active Regions and Coronal Holes to be thresholded according to their Raw area instead of their center area.\n\t", thresholdRawArea);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.new_flag('u', "uncompressed_results", "\n\tSet this flag if you want results maps to be uncompressed.\n\t", uncompressed_results);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\tThe order is important, the first image will be used as a reference for time, suncenter,... and will be used for the statistics.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version);
	arguments.process(argc, argv);

	
	// General variables
	vector<RealFeature> B;
	vector<vector<RealFeature> > Bs;
	RealFeature wavelengths = 0;
	Classifier* F;
	RealFeature binSize(0);

	bool classifierIsPossibilistic = false;
	bool classifierIsHistogram = false;

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
	
	// We read the wavelengths and the initial class centers from the centers file
	if(isFile(centersFileName))
	{
		readCentersFromFile(Bs, wavelengths, centersFileName);
		if(Bs.size() > 0)
		{
			B = median_classcenters(Bs);
			#if DEBUG >= 3
			cout<<"Bmedian "<<B<<endl;
			for (unsigned b = 0; b < Bs.size(); ++b)
				cout<<"Bs"<<b<<" "<<Bs[b]<<endl;
			#endif
		}
		if(B.size() > 0 && B.size() != numberClasses)
		{
			cerr<<"Error : The number of classes is different than the number of centers read in the center file."<<endl;
			numberClasses = B.size();
			cerr<<"The number of classes will be set to "<<numberClasses<<endl;
		
		}
	}
	
	// We read the bin size
	if(!sbinSize.empty())
	{
		sbinSize>>binSize;
	}

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
	else if (classifierType == "PFCM")
	{
		F = new PFCMClassifier(fuzzifier, fuzzifier, 2, 6);
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
	else if (classifierType == "HFCM")
	{
		if(isFile(histogramFile))
		{
			F = new HistogramFCMClassifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
		{
			F = new HistogramFCMClassifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
		classifierIsHistogram = true;
	}
	else if (classifierType == "HPCM")
	{

		if(isFile(histogramFile))
		{
			F = new HistogramPCMClassifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
		{
			F = new HistogramPCMClassifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
		classifierIsHistogram = true;
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "HPCM2")
	{

		if(isFile(histogramFile))
		{
			F = new HistogramPCM2Classifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
		{
			F = new HistogramPCM2Classifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
		classifierIsHistogram = true;
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
		if(p > 0 && !(images[0]->checkSimilar(images[p])))
		{
			cerr<<"Warning: image "<<imagesFilenames[p]<<" is not similar to image "<<imagesFilenames[0]<<endl;
		}
			
	}
	
	// We set the name of the output files prefix
	// to the outputDirectory + the date_obs of the first image in the form YYYYMMDD_HHMMSS 
	if(filenamePrefix.empty())
		filenamePrefix = outputDirectory + "/" + time2string(images[0]->ObservationTime()) + ".";
	
	// We add the images to the classifier
	F->addImages(images);
	
	// We delete all images to gain memory space
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
		
		
	// If we don't have centers we initialise randomly
	if(B.size() == 0)
	{
		F->randomInitB(numberClasses);
	}
	else // We initialize the Classifier with the centers from the centers file
	{
		F->initB(B, wavelengths);
	}

	if(classifierIsPossibilistic)
	{
		dynamic_cast<PCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
		#if DEBUG >= 2
		// We save the centers we found
		F->saveB(filenamePrefix + "FCM.centers.txt");
		#endif
	}	
	
	#if DEBUG >= 3
	cout<<"The centers have been initialized to B :"<<F->getB()<<endl;
	#endif

	// We have all the information we need, we can do the classification
	F->classification(precision, maxNumberIteration);
	
	// We retrieve the new centers found
	B = F->getB();
	wavelengths = F->getChannels();
	sort(B.begin(), B.end());
	Bs.insert(Bs.begin(), B);
	
	// If we need to take into account the previous centers found
	// We adapt the centers found by the classification
	if(numberPreviousCenters > 0)
	{
		B = median_classcenters(Bs);
		if(Bs.size() > numberPreviousCenters)
			Bs.resize(numberPreviousCenters+1);
			
		#if DEBUG >= 3
		cout<<"Re-Initialized B with "<<B<<endl;
		#endif
		F->initB(B, wavelengths);
		if(classifierIsPossibilistic)
		{
				dynamic_cast<PCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
				F->initB(B, wavelengths);
		}	
	}

	// We always terminate by an attribution
	// it sorts the class centers
	// it is needed when the Quotient Factor was bad
	// or if the classifier is histogram
	F->attribution();
	
	// We save the centers for the next run 
	if (!centersFileName.empty())
	{
		writeCentersToFile(Bs, wavelengths, centersFileName);
	}
	else
	{
		writeCentersToFile(Bs, wavelengths, filenamePrefix + "centers.txt");
	}

	// We save the histogram
	if(classifierIsHistogram && !histogramFile.empty())
	{
		dynamic_cast<HistogramClassifier*>(F)->saveHistogram(histogramFile);
	}
	// We save the eta
	if(classifierIsPossibilistic)
	{
		dynamic_cast<PCMClassifier*>(F)->saveEta(filenamePrefix + "eta.txt");
	}
	
	// We check what are the requested maps
	
	bool getARMap = (desiredMaps.find_first_of("Aa")!=string::npos);
	bool getCHMap = (desiredMaps.find_first_of("Cc")!=string::npos);
	bool getSegmentedMap = (desiredMaps.find_first_of("Ss")!=string::npos);
	
	if(!(getARMap || getCHMap || getSegmentedMap))
	{
		cerr<<"Warning: No maps has been requested"<<endl;
		delete F;
		return EXIT_SUCCESS;
	}
	
	// I need the first image for the region stats
	EUVImage* image = getImageFromFile(imageType, imagesFilenames[0]);
	image->preprocessing(regionStatsPreprocessing, regionStatsRadiusRatio);
	
	// We declare the segmented map with the keywords of the first image
	ColorMap* segmentedMap = new ColorMap(image->getHeader());
	segmentedMap->copySunParameters(image);
	
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
	
	//We create an header of information about the classification
	B = F->getB();
	
	Header classification_info;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		classification_info.set<string>(string("IMAGE")+itos(p+1,3), imagesFilenames[p]);
	}

	classification_info.set<string>("CVERSION", version, "SPoCA Version");
	classification_info.set<string>("CPREPROC", preprocessingSteps, "Preprocessing Steps");
	classification_info.set<double>("CRADRATI", radiusRatio, "Radius Ratio");
	classification_info.set<string>("CLASTYPE", classifierType, "Classifier Type");
	classification_info.set<int>("CNBRCLAS", numberClasses, "Number Classes");
	classification_info.set<double>("CPRECIS", precision, "Classifier Precision");
	classification_info.set<int>("CMAXITER", maxNumberIteration, "Max Number Iteration");
	classification_info.set<double>("CFUZFIER", fuzzifier, "Classifier Fuzzifier");
	classification_info.set<string>("CBINSIZE", sbinSize, "Histogram bin size");
	classification_info.set<string>("RPREPROC", regionStatsPreprocessing, "Region Stats Preprocesing");
	classification_info.set<double>("RRADRATI", regionStatsRadiusRatio, "Region Stats Radius Ratio");

	classification_info.set<string>("SEGMTYPE", segmentation, "Segmentation type");
	if(! activeRegion.empty())
		classification_info.set<string>("SFIXAR", activeRegion);
	if(! quietSun.empty())
		classification_info.set<string>("SFIXQS", quietSun);
	if(! coronalHole.empty())
		classification_info.set<string>("SFIXCH", coronalHole);
	if(! threshold.empty())
		classification_info.set<string>("STRSHLD", threshold);
	classification_info.set<int>("MRAWTRSH", thresholdRawArea, "Region Size Threshold on Raw Area");
	
	ostringstream ss;
	ss<<F->getChannels()<<" "<<fixed<<setprecision(3)<<B;
	classification_info.set<string>("CCENTER", ss.str(), "Classification Center");
	
	if(classifierIsPossibilistic)
	{
		ostringstream ss;
		ss<<dynamic_cast<PCMClassifier*>(F)->getEta();
		classification_info.set<string>("CETA", ss.str(), "Classification eta");
	}
	
	delete F;
	
	// We set wheter we should not compress the maps
	const int compressed_fits = uncompressed_results ? 0 : FitsFile::compress;
	
	// We get the class of AR
	unsigned ARClassNumber = 3;
	if (segmentation == "max" || segmentation == "closest" || segmentation == "limits")
	{
		ARClassNumber = ARclass(B);
	}
	
	if(getARMap)
	{
		// We get the map of AR
		ColorMap* ARMap = ActiveRegionMap(segmentedMap, ARClassNumber, thresholdRawArea);
	
		// We write the map of AR to a fits file
		FitsFile file(filenamePrefix + "ARmap.fits", FitsFile::overwrite);
		ARMap->writeFits(file, compressed_fits);
		file.writeHeader(classification_info);
		
		// We get the AR Stats
		vector<ActiveRegionStats*> ARStats = getActiveRegionStats(ARMap, image);
		
		// We write the AR Stats into the fits
		file.writeTable("ActiveRegionStats");
		writeRegions(file, ARStats);
		delete ARMap;
		
		#if DEBUG>= 3
		cerr<<"ActiveRegionStats Table"<<endl;
		if(ARStats.size() > 0)
			cerr<<ARStats[0]->toString("|", true)<<endl;
		else
			cerr<<"Empty"<<endl;
		for (unsigned r = 0; r < ARStats.size(); ++r)
		{
			cerr<<ARStats[r]->toString("|")<<endl;
		}
		#endif
		for (unsigned r = 0; r < ARStats.size(); ++r)
		{
			delete ARStats[r];
		}
	}


	// We get the class of CH
	unsigned CHClassNumber = 1;
	if (segmentation == "max" || segmentation == "closest" || segmentation == "limits")
	{
		CHClassNumber = CHclass(B);
	}
	if(getCHMap)
	{
		// We get the map of CH
		ColorMap* CHMap = CoronalHoleMap(segmentedMap, CHClassNumber, thresholdRawArea);
		
		// We write the map of CH to a fits file
		FitsFile file(filenamePrefix + "CHmap.fits", FitsFile::overwrite);
		CHMap->writeFits(file, compressed_fits);
		file.writeHeader(classification_info);
		
		// We get the CH Stats
		vector<CoronalHoleStats*> CHStats = getCoronalHoleStats(CHMap, image);
		
		// We write the CH Stats into the fits
		file.writeTable("CoronalHoleStats");
		writeRegions(file, CHStats);
		
		#if DEBUG>= 3
		cerr<<"CoronalHoleStats Table"<<endl;
		if(CHStats.size() > 0)
			cerr<<CHStats[0]->toString("|", true)<<endl;
		else
			cerr<<"Empty"<<endl;
		for (unsigned r = 0; r < CHStats.size(); ++r)
		{
			cerr<<CHStats[r]->toString("|")<<endl;
		}
		#endif
		
		//We get the CH chaincode and write them to the CH map
		file.writeTable("CoronalHoleChaincode");
		unsigned chaincode_nbr_points = 20;
		for (unsigned r = 0; r < CHStats.size(); ++r)
		{
			vector<Coordinate> CH_chaincode = CHMap->chainCode(CHStats[r]->FirstPixel(), chaincode_nbr_points);
			vector<unsigned> xdata(CH_chaincode.size(),0);
			vector<unsigned> ydata(CH_chaincode.size(),0);
			for(unsigned c = 0; c < CH_chaincode.size(); ++c)
			{
				xdata[c] = CH_chaincode[c].x;
				ydata[c] = CH_chaincode[c].y;
			}
			file.writeColumn("X"+itos(r,7), xdata);
			file.writeColumn("Y"+itos(r,7), ydata);
			
			#if DEBUG>= 3
			cerr<<"CoronalHole Chaincode for region id "<<CHStats[r]->Id()<<endl;
			cerr<<CH_chaincode<<endl;
			#endif
		}
		delete CHMap;
		
		for (unsigned r = 0; r < CHStats.size(); ++r)
		{
			delete CHStats[r];
		}
	}

	if(getSegmentedMap)
	{
		// We set the class info into the segmentedMap
		classification_info.set("ARCLASS", ARClassNumber, "Color of the Active Region class");
		classification_info.set("CHCLASS", CHClassNumber, "Color of the Coronal Hole class");
	
		// We write the segmentedMap to a fits file
		FitsFile file(filenamePrefix + "segmentedMap.fits", FitsFile::overwrite);
		segmentedMap->writeFits(file,compressed_fits);
		file.writeHeader(classification_info);
		
		// We get the RegionStats
		vector<RegionStats*> regionStats = getRegionStats(segmentedMap, image);
		
		// We write the RegionStats into the fits
		file.writeTable("RegionStats");
		writeRegions(file, regionStats);
		
		#if DEBUG>= 3
		cerr<<"RegionStats Table"<<endl;
		if(regionStats.size() > 0)
			cerr<<regionStats[0]->toString("|", true)<<endl;
		else
			cerr<<"Empty"<<endl;
		for (unsigned r = 0; r < regionStats.size(); ++r)
		{
			cerr<<regionStats[r]->toString("|")<<endl;
		}
		#endif
		
		for (unsigned r = 0; r < regionStats.size(); ++r)
		{
			delete regionStats[r];
		}
	}
	delete segmentedMap;
	delete image;

	return EXIT_SUCCESS;
}
