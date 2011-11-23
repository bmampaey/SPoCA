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
	if(images.size() < NUMBERCHANNELS)
	{
		cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
		exit(EXIT_FAILURE);
	}
	else if(images.size() > NUMBERCHANNELS)
	{
		cerr<<"Warning : The number of images is not equal to "<<NUMBERCHANNELS<<". Only using the first ones."<<endl;
	}
	
	// We must verify that the channels of the histogram are the same as the channels of the classifier
	if(channels.size() > 0)
	{
		if(histoChannels.empty())
		{
			histoChannels = channels;
		}
		else if(histoChannels.size() != channels.size()
		{
			cerr<<"Error : number of channels of the histogram differ from the number of channels of the classifier, check centers file or histogram file."<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			for(unsigned p = 0; p < channels.size(); ++p)
			{
				if(histoChannels[p] != channels[p])
				{
					cerr<<"Error : channel "<<p<<" of the histogram ("<<histoChannels[p]<<") differ from classifier ("<<channels[p]<<"), check centers file or histogram file."<<endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	else 
	{
		if(histoChannels.empty())
		{
			histoChannels.resize(NUMBERCHANNELS);
			for (unsigned p = 0; p< NUMBERCHANNELS; ++p)
				histoChannels[p] = images[p]->Channel();
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
	saveHistogram(filenamePrefix + itos(numberImages) + "images.histogram.txt");
	#endif

}
