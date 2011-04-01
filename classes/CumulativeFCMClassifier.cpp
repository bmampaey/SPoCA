#include "CumulativeFCMClassifier.h"

using namespace std;

CumulativeFCMClassifier::CumulativeFCMClassifier(Real fuzzifier)
:HistogramFCMClassifier(), CumulativeClassifier()
{
	this->fuzzifier = fuzzifier;
}

CumulativeFCMClassifier::CumulativeFCMClassifier(const RealFeature& binSize, Real fuzzifier)
:HistogramFCMClassifier(), CumulativeClassifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);
}

CumulativeFCMClassifier::CumulativeFCMClassifier(const std::string& histogramFilename, Real fuzzifier)
:HistogramFCMClassifier(), CumulativeClassifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
}


void CumulativeFCMClassifier::addImages(std::vector<EUVImage*> images)
{

	for (unsigned p = 0; p <  NUMBERWAVELENGTH; ++p)
	{
		if( binSize.v[p] == 0 )
		{
			cerr<<"binSize cannot be 0."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	
	if(channels)
	{
		if(!histoChannels)
		{
			histoChannels = channels;
		}
		else if(histoChannels != channels)
		{
			cerr<<"Error : channels in the histogram file do not correspond to channels of the classifier (check centers file or order of the images)."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	else 
	{
		if(!histoChannels)
		{
			for (unsigned p = 0; p< NUMBERWAVELENGTH; ++p)
				histoChannels.v[p] = images[p]->Wavelength();
		}
		channels = histoChannels;
	}


	checkImages(images);
	ordonateImages(images);

	unsigned xaxes = images[0]->Xaxes();
	unsigned yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p <  NUMBERWAVELENGTH; ++p)
	{
		xaxes = images[p]->Xaxes() < xaxes ? images[p]->Xaxes() : xaxes;
		yaxes = images[p]->Yaxes() < yaxes ? images[p]->Yaxes() : yaxes;
	}
	if(Xaxes == 0)
	{
		Xaxes = xaxes;
	}
	#if DEBUG >= 1
	if(xaxes != Xaxes)
	{
		cerr<<"Error : All the images must have the same Xaxes size."<<endl;
		exit(EXIT_FAILURE);
	}
	#endif


	HistoPixelFeature xj;
	bool validPixel;
	for (unsigned y = 0; y < yaxes; ++y)
	{
		for (unsigned x = 0; x < xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERWAVELENGTH && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(x, y);
				if(xj.v[p] == images[p]->nullvalue())
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

	numberBins = HistoX.size();
	Yaxes += yaxes;
	++numberImages;

	#if DEBUG >= 2
	saveHistogram(outputFileName + itos(numberImages) + "image.histogram.txt");
	#endif

}
