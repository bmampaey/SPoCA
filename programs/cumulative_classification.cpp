// This programm will do cumulative classification on sun images
// Written by Benjamin Mampaey on 14 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"

#include "../classes/SunImage.h"

#include "../classes/CumulativeClassifier.h"
#include "../classes/CumulativeFCMClassifier.h"
#include "../classes/CumulativePCM2Classifier.h"
#include "../classes/CumulativePCMClassifier.h"
#include "../classes/CumulativeSPoCA2Classifier.h"
#include "../classes/CumulativeSPoCAClassifier.h"

#include "../classes/FeatureVector.h"
#include "../classes/ArgumentHelper.h"



using namespace std;
using namespace dsr;

string outputFileName;

int main(int argc, const char **argv)
{
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

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
	string classifierType;
	unsigned maxNumberIteration = 100;
	double precision = 0.001;
	double fuzzifier = 2;
	unsigned classificationPeriodicity = 1;
	bool reinit = false;
		
	// Option for the Spacial Classifiers (SPoCA)
	unsigned neighboorhoodRadius = 1;
	
	// Options for the histogram classifiers
	string sbinSize;
	string histogramFile;
	

	// We parse the arguments

	string programDescription = "This program does cumulative classification.\n";
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
	arguments.new_named_string('B',"centersFile","file name", "\n\tThe name of the file containing the centers.\n\tIf it it not provided the centers will be initialized randomly.\n\t", centersFileName);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_unsigned_int('N', "neighboorhoodRadius", "positive integer", "\n\tOnly for spatial classifiers like SPoCA.\n\tThe neighboorhoodRadius is half the size of the square of neighboors, for example with a value of 1, the square has a size of 3x3.\n\t", neighboorhoodRadius);
	arguments.new_named_string('H', "histogramFile","file name", "\n\tThe name of a file containing an histogram.\n\t", histogramFile);
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogramm.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_unsigned_int('t', "classificationPeriodicity", "classificationPeriodicity", "The periodicity with wich we do the classification (0 means only classification at the end).\n\t", classificationPeriodicity);
	arguments.new_flag('R', "reinitializeCenters", "Set this flag if you want the centers to be reinitialized before each classification (For possibilistic classifiers this mean doing a FCM init again.\n\t", reinit);
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

	// We process the arguments

	// We assert that the number of sun images provided is correct
	if(sunImagesFileNames.size() % NUMBERWAVELENGTH != 0)
	{
		cerr<<"Error : "<<sunImagesFileNames.size()<<" fits image file given as parameter, a multiple of "<<NUMBERWAVELENGTH<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}


	// We set the name of output files
	// If none as been provided as a program argument, we set it to the current directory + type of classifier
	if(outputFileName.empty())
	{
		outputFileName = "./Cumulative" + classifierType;
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
		if(fileExists(histogramFile))
		{
			F = new CumulativeFCMClassifier(histogramFile, fuzzifier);
		}
		else if(binSize)
		{
			F = new CumulativeFCMClassifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
	}
	else if (classifierType == "PCM")
	{
		if(fileExists(histogramFile))
		{
			F = new CumulativePCMClassifier(histogramFile, fuzzifier);
		}
		else if(binSize)
		{
			F = new CumulativePCMClassifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "PCM2")
	{
		if(fileExists(histogramFile))
		{
			F = new CumulativePCM2Classifier(histogramFile, fuzzifier);
		}
		else if(binSize)
		{
			F = new CumulativePCM2Classifier(binSize, fuzzifier);
		}
		else
		{
			cerr<<"Error : for histogram classification you must provide the bin size or an histogram file."<<endl;
			return EXIT_FAILURE;
		}
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "SPoCA")
	{
		F = new CumulativeSPoCAClassifier(neighboorhoodRadius, fuzzifier);
		classifierIsPossibilistic = true;
	}
	else if (classifierType == "SPoCA2")
	{
		F = new CumulativeSPoCA2Classifier(neighboorhoodRadius, fuzzifier);
		classifierIsPossibilistic = true;
	}
	else 
	{
		cerr<<"Error : "<<classifierType<<" is not a known classifier!"<<endl;
		return EXIT_FAILURE;
	}
	
	string outputFileNameBase = outputFileName;
	bool firstinit = true;
	unsigned M = sunImagesFileNames.size() / NUMBERWAVELENGTH;
	vector<string> subSunImagesFileNames(NUMBERWAVELENGTH);
	
	ofstream outputFile((outputFileName + "cumulative_output.txt").c_str());
	
	for(unsigned m = 0 ; m < M; ++m )
	{
		// We set the outputFileName
		outputFileName = outputFileNameBase + "m" + itos(m + 1) + ".";

		// We read and preprocess a tuple sun images
		for (unsigned p = 0; p <  NUMBERWAVELENGTH; ++p)
			subSunImagesFileNames[p] = sunImagesFileNames[m * NUMBERWAVELENGTH + p];
			
		vector<SunImage*> images = getImagesFromFiles(imageType, subSunImagesFileNames, true);
		
		for (unsigned p = 0; p < images.size(); ++p)
		{
			images[p]->preprocessing(preprocessingSteps, radiusRatio);
			#if defined(DEBUG) && DEBUG >= 2
			images[p]->writeFitsImage(outputFileNameBase + "preprocessed." + sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0));
			#endif
		}
		
		
		// We save the names of the files added
		outputFile<<"Adding images: ";
		for (unsigned p = 0; p< NUMBERWAVELENGTH; ++p)
			outputFile<<subSunImagesFileNames[p]<<" ";
		outputFile<<endl;


		// We add the images to the classifier
		F->addImages(images);

		//We can save memory now by deleting the images
		for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
			delete images[p];

		if( (classificationPeriodicity > 0 && (m%classificationPeriodicity == 0) ) || m == M - 1 || M == 1)
		{
			// We initialize the centers
			if(firstinit || reinit)
			{
				firstinit = false;
				// If we don't have centers we initialise randomly
				if(B.size() == 0)
				{
					F->randomInit(numberClasses);
				}	
				else // We initialize the Classifier with the centers from the centers file
				{
					F->init(B, wavelengths);
				}

				if(classifierIsPossibilistic)
				{
					dynamic_cast<CumulativePCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
				}	
			}
	
			// Everything s ready, we do the classification
			F->classification(precision, maxNumberIteration);
			// We save the centers
			outputFile<<"m: "<< m + 1<<"\tB: "<<F->getB();
			if(classifierIsPossibilistic)
				outputFile<<"\teta: "<<dynamic_cast<CumulativePCMClassifier*>(F)->getEta();
			cout<<endl;
			
		}

	}
	outputFile.close();
	
	return EXIT_SUCCESS;

}
