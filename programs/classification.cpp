// This programm will do classification and segmentation on sun images
// Written by Benjamin Mampaey on 1 July 2010


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

#include "../classes/Classifier.h"
#include "../classes/FCMClassifier.h"
#include "../classes/PCMClassifier.h"
#include "../classes/PCM2Classifier.h"
#include "../classes/SPoCAClassifier.h"
#include "../classes/SPoCA2Classifier.h"
#include "../classes/HistogramFCMClassifier.h"
#include "../classes/HistogramPCM2Classifier.h"
#include "../classes/HistogramPCMClassifier.h"

#include "../classes/FeatureVector.h"
#include "../classes/ArgumentHelper.h"



using namespace std;
using namespace dsr;



string outputFileName;

int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);

	// The list of names of the sun images to process
	string imageType = "AIA";
	vector<string> sunImagesFileNames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

	// The number of classes to classify into
	unsigned numberClasses = 0;
	
	// Option to pass a class center file
	string centersFileName;

	// Options for the classification
	string classifierType = "HFCM";
	unsigned maxNumberIteration = 100;
	double precision = 0.001;
	double fuzzifier = 2;
		
	// Option for the Spacial Classifiers (SPoCA)
	unsigned neighboorhoodRadius = 1;
	
	// Options for the histogram classifiers
	string sbinSize;
	string histogramFile;
	

	// We parse the arguments

	string programDescription = "This Programm does classification and segmentation.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('T',"classifierType", "string", "\n\tThe type of classifier to use for the classification.\n\tPossible values are : FCM, PCM, PCM2, SPOCA, SPOCA2, HFCM(Histogram FCM), HPCM(Histogram PCM), HPCM2(Histogram PCM2)\n\t", classifierType);
	arguments.new_named_unsigned_int('i', "maxNumberIteration", "positive integer", "\n\tThe maximal number of iteration for the classification.\n\t", maxNumberIteration);
	arguments.new_named_double('p',"precision", "positive real", "\n\tThe precision to be reached to stop the classification.\n\t",precision);
	arguments.new_named_double('f',"fuzzifier", "positive real", "\n\tThe fuzzifier (m).\n\t",fuzzifier);
	arguments.new_named_unsigned_int('C', "numberClasses", "positive integer", "\n\tThe number of classes to classify the sun images into.\n\t", numberClasses);
	arguments.new_named_string('B',"centersFile","file name", "\n\tThe name of the file containing the centers.\n\tIf it it not provided the centers will be initialized randomly.\n\t", centersFileName);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_unsigned_int('N', "neighboorhoodRadius", "positive integer", "\n\tOnly for spatial classifiers like SPoCA.\n\tThe neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3.\n\t", neighboorhoodRadius);
	arguments.new_named_string('H', "histogramFile","file name", "\n\tThe name of a file containing an histogram.\n\t", histogramFile);
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogramm.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", sunImagesFileNames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("2.0");
	arguments.process(argc, argv);

	
	// General variables
	vector<RealFeature> B;
	RealFeature wavelengths = 0;
	Classifier* F;
	RealFeature binSize(0);

	bool classifierIsPossibilistic = false;
	bool classifierIsHistogram = false;

	// We process the arguments

	// We assert that the number of sun images provided is correct
	if(sunImagesFileNames.size() != NUMBERWAVELENGTH)
	{
		cerr<<"Error : "<<sunImagesFileNames.size()<<" fits image file given as parameter, "<<NUMBERWAVELENGTH<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}


	// We set the name of output files
	// If none as been provided as a program argument, we set it to the current directory + type of classifier
	if(outputFileName.empty())
	{
		outputFileName = "./" + classifierType;
	}
	outputFileName += ".";
	
	// We read the wavelengths and the initial centers from the centers file
	if(readCentersFromFile(B, wavelengths, centersFileName))
	{
		if(B.size() != numberClasses)
		{
			cerr<<"Error : The number of classes is different than the number of centers read in the center file."<<endl;
			numberClasses = B.size();
			cerr<<"The number of classes will be set to "<<numberClasses<<endl;
			
		}
	}
	
	
	// We read the bin size
	if(!readbinSize(binSize,sbinSize))
	{
		return EXIT_FAILURE;
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
	else if (classifierType == "HFCM")
	{
		if(fileExists(histogramFile))
		{
			F = new HistogramFCMClassifier(histogramFile, fuzzifier);
		}
		else if(binSize)
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

		if(fileExists(histogramFile))
		{
			F = new HistogramPCMClassifier(histogramFile, fuzzifier);
		}
		else if(binSize)
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

		if(fileExists(histogramFile))
		{
			F = new HistogramPCM2Classifier(histogramFile, fuzzifier);
		}
		else if(binSize)
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
	vector<SunImage*> images = getImagesFromFiles(imageType, sunImagesFileNames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
		#if DEBUG >= 2
		images[p]->writeFitsImage(outputFileName + "preprocessed." + sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0));
		#endif
	}

	
	// We add the images to the classifier
	F->addImages(images);
		
	// We delete all images but the first one to gain memory space
	for (unsigned p = 1; p < images.size(); ++p)
	{
		delete images[p];
	}
	images.resize(1);
		
		
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
		F->saveB(outputFileName + "FCM.centers.txt");
		#endif
	}	
	
	#if DEBUG >= 3
	cout<<"The centers have been initialized to B :"<<F->getB()<<endl;
	#endif

	// We have all the information we need, we can do the classification
	F->classification(precision, maxNumberIteration);

	#if DEBUG >= 2
	// We save the results
	F->saveAllResults(images[0]);
	#endif
	
	// We save the map of AR
	F->saveARmap(images[0]);
	
	//We save the centers for the next run (for PCM and derivatives, must be done before classification)
	if (!centersFileName.empty())
	{
		F->saveB(centersFileName);
	}

	//We save the histogram for the next run
	if(classifierIsHistogram && !histogramFile.empty())
	{
		dynamic_cast<HistogramClassifier*>(F)->saveHistogram(histogramFile);
	}
	if(classifierIsPossibilistic)
	{
		dynamic_cast<PCMClassifier*>(F)->saveEta(outputFileName + "eta.txt");
	}
	
	delete F;
	delete images[0];

	return EXIT_SUCCESS;
}
