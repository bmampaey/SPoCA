//! Program that does cumulative classification of EUV sun images
/*!
@page cumulative_classification cumulative_classification.x

 This program takes a list of tuples of EUV sun images in fits format, does the requested classification.
 
 A tuple of images is a list of images that have different channels / wavelength but are similar.
 
 It outputs only the centers of classes, and does not do segmentation.
 
 
 @section usage Usage
 
 <tt> cumulative_classification.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> cumulative_classification.x [-option optionvalue, ...] fitsFileName1.1 fitsFileName1.2 fitsFileName2.1 fitsFileName2.2 ...</tt>
 
 You must provide exactly the same number of fits file per channel.
 The order of the fits files is important, you must group the tuples together.
 
 
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
 If it it not provided the centers will be initialized randomly.

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

@param histogramFile	The name of a file containing an histogram.

@param binSize	The size of the bins of the histogram.
<BR>N.B. Be carreful that the histogram is built after the preprocessing.

@param classificationPeriodicity	The periodicity with wich we do the classification (0 means only classification at the end)
@param reinitializeCenters	Set this flag if you want the centers to be reinitialized before each classification (For possibilistic classifiers this mean doing a FCM init again)

@param outputFile	The name for the output file(s)

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

#include "../classes/EUVImage.h"

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

string filenamePrefix;

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
	string classifierType;
	unsigned maxNumberIteration = 100;
	double precision = 0.0015;
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
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('T',"classifierType", "string", "\n\tThe type of classifier to use for the classification.\n\tPossible values are : FCM, PCM, PCM2, SPoCA, SPoCA2\n\t", classifierType);
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
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogram.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_unsigned_int('t', "classificationPeriodicity", "classificationPeriodicity", "The periodicity with wich we do the classification (0 means only classification at the end).\n\t", classificationPeriodicity);
	arguments.new_flag('R', "reinitializeCenters", "\n\tSet this flag if you want the centers to be reinitialized before each classification (For possibilistic classifiers this mean doing a FCM init again.\n\t", reinit);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", filenamePrefix);
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
	RealFeature binSize(0);

	bool classifierIsPossibilistic = false;

	// We process the arguments

	// We assert that the number of sun images provided is correct
	if(imagesFilenames.size() % NUMBERCHANNELS != 0)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file given as parameter, a multiple of "<<NUMBERCHANNELS<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}


	// We set the name of output files
	// If none as been provided as a program argument, we set it to the current directory + type of classifier
	if(filenamePrefix.empty())
	{
		filenamePrefix = "./Cumulative" + classifierType;
	}
	filenamePrefix += ".";
	
	// We read the channels and the initial centers from the centers file
	if(readCentersFromFile(B, channels, centersFileName))
	{
		if(B.size() != numberClasses)
		{
			cerr<<"Error : The number of classes is different than the number of centers read in the center file."<<endl;
			numberClasses = B.size();
			cerr<<"The number of classes will be set to "<<numberClasses<<endl;
			
		}
	}
	
	
	// We read the bin size
	if(! readbinSize(binSize, sbinSize))
	{
		cerr<<"Error reading bin size: "<<sbinSize<<endl;
		return EXIT_FAILURE;
	}

	
	// We declare the type of Classifier we want
	if (classifierType == "FCM")
	{
		if(isFile(histogramFile))
		{
			F = new CumulativeFCMClassifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
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
		if(isFile(histogramFile))
		{
			F = new CumulativePCMClassifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
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
		if(isFile(histogramFile))
		{
			F = new CumulativePCM2Classifier(histogramFile, fuzzifier);
		}
		else if(! binSize.has_null())
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
	
	string filenamePrefixBase = filenamePrefix;
	bool firstinit = true;
	unsigned M = unsigned(imagesFilenames.size() / NUMBERCHANNELS);
	vector<string> imagesFileNameSet(NUMBERCHANNELS);
	
	ofstream outputFile((filenamePrefix + "cumulative_output.txt").c_str());
	
	for(unsigned m = 0 ; m < M; ++m )
	{
		// We set the filenamePrefix
		filenamePrefix = filenamePrefixBase + "m" + itos(m + 1) + ".";

		// We read and preprocess a set of images
		for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
			imagesFileNameSet[p] = imagesFilenames[m * NUMBERCHANNELS + p];
			
		vector<EUVImage*> images = getImagesFromFiles(imageType, imagesFileNameSet, true);
		
		for (unsigned p = 0; p < images.size(); ++p)
		{
			images[p]->preprocessing(preprocessingSteps, radiusRatio);
			#if defined DEBUG
			images[p]->writeFits(filenamePrefixBase + "preprocessed." + stripPath(imagesFilenames[p]) );
			#endif
			// We check if the images are similars
			string dissimilarity = checkSimilar(images[0], images[p]);
			if(! dissimilarity.empty())
			{
				cerr<<"Warning: image "<<imagesFilenames[p]<<" and "<<imagesFilenames[0]<<" are not similar: "<<dissimilarity<<endl;
			}
		}
		
		// We save the names of the files added
		outputFile<<"Adding images: ";
		for (unsigned p = 0; p< NUMBERCHANNELS; ++p)
			outputFile<<imagesFileNameSet[p]<<" ";
		outputFile<<endl;


		// We add the images to the classifier
		F->addImages(images);

		//We can save memory now by deleting the images
		for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
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
					F->randomInitB(numberClasses);
				}	
				else // We initialize the Classifier with the centers from the centers file
				{
					F->initB(B, channels);
				}
				
				if(classifierIsPossibilistic)
				{
					if (classifierType == "SPoCA" || classifierType == "SPoCA2")
					{
						dynamic_cast<PCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
					}
					else
					{
						dynamic_cast<CumulativePCMClassifier*>(F)->FCMinit(precision, maxNumberIteration);
					}
				}	
			}
			// Everything is ready, we do the classification
			F->classification(precision, maxNumberIteration);
			// We save the centers
			outputFile<<"m: "<< m + 1<<"\tB: "<<F->getB();
			if(classifierIsPossibilistic)
			{
				if (classifierType == "SPoCA" || classifierType == "SPoCA2")
				{
					outputFile<<"\teta: "<<dynamic_cast<PCMClassifier*>(F)->getEta();
				}
				else
				{
					outputFile<<"\teta: "<<dynamic_cast<CumulativePCMClassifier*>(F)->getEta();
				}
			}
			outputFile<<endl;
			
		}

	}
	outputFile.close();
	delete F;
	return EXIT_SUCCESS;

}
