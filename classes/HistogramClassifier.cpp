#include "HistogramClassifier.h"
#include <math.h>

using namespace std;

extern string filenamePrefix;

HistogramClassifier::HistogramClassifier()
:binSize(0),numberBins(0)
{}

HistogramClassifier::HistogramClassifier(const RealFeature& binSize)
:binSize(binSize),numberBins(0)
{}

HistogramClassifier::HistogramClassifier(const std::string& histogramFilename)
:binSize(0),numberBins(0)
{
	initHistogram(histogramFilename);
}

// Function to insert a new HistoFeatureVector into HistoX
inline void HistogramClassifier::insert(const HistoRealFeature& x)
{
	pair<HistoFeatureVectorSet::iterator,bool> ret = HistoX.insert(x);
	if(! ret.second)
	{
		//The element existed already, I increase it's count
		(ret.first)->c += x.c;
	}
}

// Function to insert a new FeatureVector into HistoX
inline void HistogramClassifier::insert(const RealFeature& x)
{
	pair<HistoFeatureVectorSet::iterator,bool> ret = HistoX.insert(x);
	(ret.first)->c += 1;
}


void HistogramClassifier::initHistogram(const std::string& histogramFilename, bool reset)
{
	ifstream histoFile(histogramFilename.c_str());
	stringstream histoStream;
	vector<char> buffer;
	//We put the file into a stringstream for rapidity
	if (histoFile.good())
	{
		// We get the size of the file   
		histoFile.seekg(0,ios::end);
		streampos length = histoFile.tellg();
		histoFile.seekg(0,ios::beg);
		buffer.resize(length);

		//We read the whole file into the buffer.
		histoFile.read(&buffer[0],length);

		// We create the string stream.
		histoStream.rdbuf()->pubsetbuf(&buffer[0],length);

	}
	else
	{
		cerr<<"Error : file "<<histogramFilename<<" not found."<<endl;
		return;
	}
	histoFile.close();
	
	//We initialise the histogram
	histoStream>>histoChannels;
	histoStream>>binSize;
	histoStream>>numberBins;
	
	HistoRealFeature x;
	if (!reset)
	{
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			histoStream>>x;
			insert(x);
		}
	}
	else
	{
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			histoStream>>x;
			x.c = 0;
			insert(x);
		}
		

	}
	
}

void HistogramClassifier::initBinSize(const RealFeature& binSize)
{
	this->binSize = binSize;
}

void HistogramClassifier::saveHistogram(const std::string& histogramFilename)
{
	ofstream histoFile(histogramFilename.c_str());
	if (histoFile)
	{
		//We save the binSize and the number of bins
		histoFile<<histoChannels<<" ";
		histoFile<<binSize<<" ";
		histoFile<<HistoX.size()<<endl;
		
		//We save the Histogram
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end() && histoFile.good(); ++xj)
		{
			histoFile<<*xj<<endl;
		}

	}
	else
	{
		cerr<<"Error : Could not open file "<<histogramFilename<<" for writing."<<endl;

	}
	

	histoFile.close();
}

void HistogramClassifier::addFeatures(const FeatureVectorSet& X)
{

	if(binSize.has_null() )
	{
		cerr<<"binSize cannot be 0."<<endl;
		exit(EXIT_FAILURE);
	}

	//TODO: test if it is faster to use the other insert
	HistoRealFeature f;
	f.c = 1;
	for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
	{
		for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
		{
			f.v[p] = (floor(xj->v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
		}

		insert(f);
	}

	numberBins = HistoX.size();
	
	#if DEBUG >= 2
	saveHistogram(filenamePrefix + "histogram.txt");
	#endif

}


void HistogramClassifier::addImages(vector<EUVImage*> images, const unsigned xaxes, const unsigned yaxes)
{

	if(binSize.has_null() )
	{
		cerr<<"binSize cannot be 0."<<endl;
		exit(EXIT_FAILURE);
	}

	HistoRealFeature f;
	f.c = 1;

	for (unsigned y = 0; y < yaxes; ++y)
	{
		for (unsigned x = 0; x < xaxes; ++x)
		{
			bool validPixel = true;
			for (unsigned p = 0; p < NUMBERCHANNELS && validPixel; ++p)
			{
				f.v[p] = images[p]->pixel(x, y);
				if(f.v[p] == images[p]->null())
					validPixel=false;
				else
					f.v[p] = (floor(f.v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
			}
			if(validPixel)
			{
				insert(f);
			}
		}
	}

	numberBins = HistoX.size();

}


