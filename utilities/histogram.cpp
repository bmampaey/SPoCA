// This programm will generate a histogram file
// Written by Benjamin Mampaey on 14 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
# include <algorithm>

#include "../classes/SunImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"
#include "../classes/HistogramFeatureVector.h"
#include "../classes/FeatureVector.h"

using namespace std;
using namespace dsr;

string outputFileName;

vector<HistoPixelFeature> HistoX;

unsigned insert(const HistoPixelFeature& xj)
{
	unsigned bsup = HistoX.size();
	unsigned binf = 0;
	unsigned pos = 0;
	while(binf < bsup)
	{
		pos = unsigned((bsup+binf)/2);
		switch(compare(HistoX[pos], xj))
		{
			case -1 :
				binf = pos + 1;
				break;
			case 1 :
				bsup = pos;
				break;
			default :
				return pos;

		}
	}
	if (bsup == HistoX.size())
	{
		HistoX.push_back(xj);
	}
	else
	{

		vector<HistoPixelFeature>::iterator there = HistoX.begin();
		there += bsup;
		HistoX.insert(there,xj);
	}
	return bsup;

}

bool cmp (const HistoPixelFeature& x1, const HistoPixelFeature& x2)
{
	return compare(x1,x2) < 0 ? true : false; 
}

void histogram(const vector<SunImage*>& images, RealFeature binSize)
{
	HistoX.reserve(images[0]->numberValidPixelsEstimate());
	HistoPixelFeature xj;
	bool validPixel;
	
	if(binSize)
	{
		for (unsigned j = 0; j < images[0]->NumberPixels(); ++j)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERWAVELENGTH && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(j);
				if(xj.v[p] == images[p]->nullvalue)
					validPixel=false;
				else 
					xj.v[p] = (int(xj.v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
			}
			if(validPixel)
			{
				unsigned pos = insert(xj);
				++HistoX[pos].c;
			}

		
		}
	}
	else // Case no binsize has been given, we output 1 bin per pixel
	{
		xj.c = 1;
		for (unsigned j = 0; j < images[0]->NumberPixels(); ++j)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERWAVELENGTH && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(j);
				if(xj.v[p] == images[p]->nullvalue)
					validPixel=false;
			}
			if(validPixel)
			{
				HistoX.push_back(xj);
			}

		
		}
		sort(HistoX.begin(),HistoX.end(),cmp);
	}
	
}


int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// Options for the preprocessing of images
	string imageType = "AIA";
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

	// The list of names of the sun images to process
	vector<string> sunImagesFileNames;

	// Options for the histogram classifiers
	string sbinSize;

	
	outputFileName = "histogram.txt";

	string programDescription = "This Program will generate the histogram file for the given images.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('z', "binSize","comma separated list of positive real (no spaces)", "\n\tThe size of the bins of the histogramm.\n\tNB : Be carreful that the histogram is built after the preprocessing.\n\t", sbinSize);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", sunImagesFileNames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	vector<SunImage*> images;
	RealFeature binSize(0);

	if(sunImagesFileNames.size() != NUMBERWAVELENGTH)
	{
		cerr<<"Error : "<<sunImagesFileNames.size()<<" fits image file given as parameter, "<<NUMBERWAVELENGTH<<" must be given!"<<endl;
		return EXIT_FAILURE;
	}

	images = getImagesFromFiles(imageType, sunImagesFileNames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
	}

	// We read the histogram bin size
	if(!sbinSize.empty())
	{
		istringstream Z(sbinSize);
		Z>>binSize;
		if(Z.fail())
		{
			cerr<<"Error reading the binSize."<<endl;
			return EXIT_FAILURE;
		}
	}
	
	RealFeature histoChannels;
	for (unsigned p = 0; p< NUMBERWAVELENGTH; ++p)
			histoChannels.v[p] = images[p]->Wavelength();
			
			
	histogram(images, binSize);
	
	ofstream histoFile(outputFileName.c_str());
	if (histoFile)
	{
		//We save the binSize and the number of bins
		histoFile<<histoChannels<<" ";
		histoFile<<binSize<<" ";
		histoFile<<HistoX.size()<<endl;
		
		//We save the Histogram
		for (unsigned j = 0; j < HistoX.size() && histoFile.good(); ++j)
		{
			for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
				histoFile<<HistoX[j].v[p]<<" ";
			histoFile<<HistoX[j].c<<endl;
		}

	}
	else
	{
		cerr<<"Error : Could not open file "<<outputFileName<<" for writing."<<endl;

	}
	

	histoFile.close();

	return EXIT_SUCCESS;
}
