// This program will do attribution (fix center classification) and segmentation on sun images
// Written by Benjamin Mampaey on 4 August 2010


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"

#include "../classes/SunImage.h"
#include "../classes/ColorMap.h"

#include "../classes/Classifier.h"
#include "../classes/FCMClassifier.h"
#include "../classes/PCMClassifier.h"
#include "../classes/PCM2Classifier.h"
#include "../classes/SPoCAClassifier.h"
#include "../classes/SPoCA2Classifier.h"


#include "../classes/FeatureVector.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ActiveRegion.h"

using namespace std;
using namespace dsr;



string outputFileName;

int main(int argc, const char **argv)
{

	cout<<setiosflags(ios::fixed);


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
	double precision = 0.001;
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
	
	// Options for the treshold segmentation
	string treshold;
	
	// Options for the ARmap
	bool tresholdRawArea = false;

	// We parse the arguments

	string programDescription = "This Programm does classification and segmentation.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
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
	arguments.new_named_string('S', "segmentation", "string", "\n\tThe segmentation type.\n\tPossible values :\n\t\tmax (Maximum of Uij)\n\t\tclosest (Closest center)\n\t\ttreshold (Treshold on Uij)\n\t\tlimits (Merge on centers value limits)\n\t\tfix (Merge on fix CH QS AR)\n\t", segmentation);
	arguments.new_named_string('L',"maxLimitsFile","file", "\n\tOnly for limit segmentation.\n\tThe name of the file containing the max limits.\n\t", maxLimitsFileName);
	arguments.new_named_string('c',"ch","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Coronal Hole.\n\t", coronalHole);
	arguments.new_named_string('q',"qs","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Quiet Sun.\n\t", quietSun);
	arguments.new_named_string('a',"ar","coma separated list of positive integer (no spaces)", "\n\tOnly for fix segmentation.\n\tThe classes of the Active Region.\n\t", activeRegion);
	arguments.new_named_string('t',"tr","coma separated list of positive integer (no spaces)", "\n\tOnly for treshold segmentation.\n\tThe parameter of the treshold segmentation.\n\tMust be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership\n\t", treshold);
	arguments.new_flag('w', "tresholdRawArea", "\n\tSet this flag if you want the Active Regions to be tresholded according to their Raw area instead of their center area.\n\t", tresholdRawArea);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("2.0");
	arguments.process(argc, argv);

	
	// General variables
	vector<RealFeature> B;
	RealFeature wavelengths = 0;
	Classifier* F;
	vector<Real> eta;

	bool classifierIsPossibilistic = false;

	// We process the arguments

	// We assert that the number of sun images provided is correct
	if(imagesFilenames.size() != NUMBERWAVELENGTH)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file given as parameter, "<<NUMBERWAVELENGTH<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}


	// We set the name of output files
	// If none as been provided as a program argument, we set it to the current directory + type of classifier
	if(outputFileName.empty())
	{
		outputFileName = "./" + classifierType + "." + segmentation;
	}
	outputFileName += ".";
	
	// We read the wavelengths and the initial centers from the centers file
	if(readCentersFromFile(B, wavelengths, centersFileName))
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
	vector<SunImage*> images = getImagesFromFiles(imageType, imagesFilenames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
		#if DEBUG >= 2
		images[p]->writeFitsImage(outputFileName + "preprocessed." + stripPath(imagesFilenames[p]) );
		#endif
	}

	
	// We add the images to the classifier
	F->addImages(images);
		
	// We declare the segmented map with the keywords of the first image
	ColorMap* segmentedMap = new ColorMap();
	segmentedMap->copyKeywords(images[0]);
		
	// We delete all images to gain memory space
	for (unsigned p = 0; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.clear();
		
		
	// We initialize the Classifier with the centers from the centers file
	F->initB(B, wavelengths);

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
			F->initB(B, wavelengths);
		}
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
	
	#if DEBUG >= 4
	// We save all the results, this should only be for advanced debugging
	F->saveAllResults(segmentedMap);
	#endif
	
	// We do the segmentation
	if (segmentation == "max")
	{
		F->segmentedMap_maxUij(segmentedMap);
	}
	else if (segmentation == "closest")
	{
		F->segmentedMap_closestCenter(segmentedMap);
	}
	else if (segmentation == "treshold")
	{
		char delimitor;
		unsigned class_number;
		Real lowerIntensity_minMembership, higherIntensity_minMembership;
		istringstream iss(treshold);
		iss>>class_number>>delimitor>>lowerIntensity_minMembership>>delimitor>>higherIntensity_minMembership;
		F->segmentedMap_classTreshold(class_number, lowerIntensity_minMembership, higherIntensity_minMembership, segmentedMap);
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
			istringstream iss(coronalHole);
			iss>>ch;
		}
		if(!quietSun.empty())
		{
			istringstream iss(quietSun);
			iss>>qs;
		}
		if(!activeRegion.empty())
		{
			istringstream iss(activeRegion);
			iss>>ar;
		}
		F->segmentedMap_fixed(ch, qs, ar, segmentedMap);
	}
	else 
	{
		cerr<<"Error : "<<segmentation<<" is not a known segmentation!"<<endl;
		return EXIT_FAILURE;
	}
	
	delete F;

	segmentedMap->writeFitsImage(outputFileName + "segmented.fits");
	
	// We save the map of AR
	if (segmentation == "max" || segmentation == "closest" || segmentation == "limits")
	{
		ActiveRegionMap(segmentedMap, ARclass(F->getB()), tresholdRawArea);
	}
	else 
	{
		ActiveRegionMap(segmentedMap, 3, tresholdRawArea);
	}
	
	segmentedMap->writeFitsImage(outputFileName + "ARmap.fits");

	/*HACK for maps with AR and CH
	for (unsigned j=0; j < segmentedMap->NumberPixels(); ++j)
	{
		if (segmentedMap->pixel(j) == 2 || segmentedMap->pixel(j) == 0)
			segmentedMap->pixel(j) = segmentedMap->nullvalue();
	}
	segmentedMap->dilateCircular(8,segmentedMap->nullvalue())->erodeCircular(8,segmentedMap->nullvalue())->dilateCircular(1,segmentedMap->nullvalue());
	unsigned minSize = 80;
	segmentedMap->tresholdConnectedComponents(20000, 1);
	segmentedMap->tresholdConnectedComponents(4166, 3);
	segmentedMap->writeFitsImage(outputFileName + "cleaned.segmented.fits");
	*/
	delete segmentedMap;	
          
	return EXIT_SUCCESS;
}
