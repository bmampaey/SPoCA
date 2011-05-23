//! Program that generates the histogram of a tuple of EUV sun images
/*!
@page histogram histogram.x

 This program takes a tuple of EUV sun images in fits format, does the requested preprocessingand generates an histogram file.
 
 A tuple of images is a list of images that have different channels / wavelength but are similar.
 
 @section usage Usage
 
 <tt> histogram.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> histogram.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
 You must provide exactly one fits file per channel.
 The order of the fits files is important, as it will set the order of the dimensions.
 
 	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogramm.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", filenamePrefix);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);

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

@param outputFile	The name of a file to write the histogram to.

@param binSize	The size of the bins of the histogramm.
<BR>N.B. Be carreful that the histogram is built after the preprocessing.

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <iostream>
#include <fstream>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <set>

#include "../classes/EUVImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"
#include "../classes/HistogramFeatureVector.h"
#include "../classes/FeatureVector.h"

using namespace std;
using namespace dsr;

string filenamePrefix;

set<HistoPixelFeature> HistoX;


// Function to insert a new HistoFeatureVector into HistoX
inline void insert(const HistoPixelFeature& xj)
{
	pair<set<HistoPixelFeature>::iterator, bool> ret = HistoX.insert(xj);
	if(! ret.second)
	{
		//The element existed already, I increase it's count
		(ret.first)->c += xj.c;
	}
}


void histogram(const vector<EUVImage*>& images, RealFeature binSize)
{
	HistoPixelFeature xj;
	xj.c = 1;
	for (unsigned j = 0; j < images[0]->NumberPixels(); ++j)
	{
		bool valid = true;
		for (unsigned p = 0; p < images.size() && valid; ++p)
		{
			valid = images[p]->pixel(j) != images[p]->nullvalue();
			xj.v[p] = (int(images[p]->pixel(j)/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
		}
		if(valid)
			insert(xj);
	}


}


int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// Options for the preprocessing of images
	string imageType = "UNKNOWN";
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	// Options for the histogram classifiers
	string sbinSize;

	
	filenamePrefix = "histogram.txt";

	string programDescription = "This program will generate the histogram file for the given images.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogramm.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", filenamePrefix);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	RealFeature binSize(0);

	if(imagesFilenames.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file given as parameter, "<<NUMBERCHANNELS<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}

	vector<EUVImage*> images = getImagesFromFiles(imageType, imagesFilenames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
	}

	// We read the histogram bin size
	// We read the bin size
	if(!readbinSize(binSize,sbinSize))
	{
		return EXIT_FAILURE;
	}
	
	RealFeature histoChannels;
	for (unsigned p = 0; p< NUMBERCHANNELS; ++p)
			histoChannels.v[p] = images[p]->Wavelength();
			
			
	histogram(images, binSize);
	
	ofstream histoFile(filenamePrefix.c_str());
	if (histoFile)
	{
		//We save the binSize and the number of bins
		histoFile<<histoChannels<<" ";
		histoFile<<binSize<<" ";
		histoFile<<HistoX.size()<<endl;
		
		//We save the Histogram
		for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end() && histoFile.good(); ++xj)
		{
			for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
				histoFile<*xj<<endl;
		}

	}
	else
	{
		cerr<<"Error : Could not open file "<<filenamePrefix<<" for writing."<<endl;

	}
	

	histoFile.close();

	return EXIT_SUCCESS;
}
