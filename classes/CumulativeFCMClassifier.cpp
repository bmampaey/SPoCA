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

	// We must verify that the channels of the histogram are the same as the channels of the classifier
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
			for (unsigned p = 0; p< NUMBERCHANNELS; ++p)
				histoChannels.v[p] = images[p]->Wavelength();
		}
		channels = histoChannels;
	}

	// We only consider the smallest common range of points from all images
	unsigned xaxes = images[0]->Xaxes();
	unsigned yaxes = images[0]->Yaxes();
	  
	for (unsigned p = 1; p <  NUMBERCHANNELS; ++p)
	{
		xaxes = images[p]->Xaxes() < xaxes ? images[p]->Xaxes() : xaxes;
		yaxes = images[p]->Yaxes() < yaxes ? images[p]->Yaxes() : yaxes;
	}
	
	// If this is the first image that we add, we set the width
	// The length shall be increased for each image we add later
	if(Xaxes == 0)
	{
		Xaxes = Xaxes == 0 ? xaxes : Xaxes;
	}
	
	#if DEBUG >= 1
	if(xaxes != Xaxes)
	{
		cerr<<"Error : All the images must have the same Xaxes size."<<endl;
		exit(EXIT_FAILURE);
	}
	#endif

	HistogramClassifier::addImages(images, xaxes, yaxes);

	Yaxes += yaxes;
	++numberImages;

	#if DEBUG >= 2
	saveHistogram(outputFileName + itos(numberImages) + "images.histogram.txt");
	#endif

}
