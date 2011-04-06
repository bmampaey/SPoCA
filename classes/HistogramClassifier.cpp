#include "HistogramClassifier.h"

using namespace std;

HistogramClassifier::HistogramClassifier()
:binSize(0),numberBins(0),histoChannels(0)
{}

HistogramClassifier::HistogramClassifier(const RealFeature& binSize)
:binSize(binSize),numberBins(0),histoChannels(0)
{}

HistogramClassifier::HistogramClassifier(const std::string& histogramFilename)
:binSize(0),numberBins(0),histoChannels(0)
{
	initHistogram(histogramFilename);
}

// Function to insert a new HistoFeatureVector into HistoX
inline void HistogramClassifier::insert(const HistoPixelFeature& xj)
{
	pair<set<HistoPixelFeature>::iterator,bool> ret = HistoX.insert(xj);
	if(! ret.second)
	{
		//The element existed already, I increase it's count
		ret.first->c += xj.c;
	}
}

// Function to insert a new FeatureVector into HistoX
inline void HistogramClassifier::insert(const PixelFeature& xj)
{
	pair<set<HistoPixelFeature>::iterator,bool> ret = HistoX.insert(xj);
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
	
	HistoPixelFeature xj;
	if (!reset)
	{
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			histoStream>>xj;
			insert(xj);
		}
	}
	else
	{
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			histoStream>>xj;
			xj.c = 1;
			insert(xj);
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
		for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end() && histoFile.good(); ++xj)
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

void HistogramClassifier::addFeatures(const vector<PixelFeature>& X)
{

	for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
	{
		if( binSize.v[p] == 0 )
		{
			cerr<<"binSize cannot be 0."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	
	//TODO: test if it is faster to use the other insert
	HistoPixelFeature xj;
	xj.c = 1;
	for (unsigned j = 0; j < X.size(); ++j)
	{
		for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
		{
			xj.v[p] = (int(X[j].v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
		}

		insert(xj);
	}

	numberBins = HistoX.size();
	
	#if DEBUG >= 2
	saveHistogram(outputFileName + "histogram.txt");
	#endif

}


void HistogramClassifier::addImages(vector<EUVImage*> images, const unsigned xaxes, const unsigned yaxes)
{

	for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
	{
		if( binSize.v[p] == 0 )
		{
			cerr<<"binSize cannot be 0."<<endl;
			exit(EXIT_FAILURE);
		}
	}


	HistoPixelFeature xj;
	xj.c = 1;
	bool validPixel;
	for (unsigned y = 0; y < yaxes; ++y)
	{
		for (unsigned x = 0; x < xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERCHANNELS && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(x, y);
				if(xj.v[p] == images[p]->nullvalue())
					validPixel=false;
				else
					xj.v[p] = (int(xj.v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
			}
			if(validPixel)
			{
				insert(xj);
			}

		}
	}

	numberBins = HistoX.size();

}


