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

// Function to insert a new FeatureVector into HistoX, if it is necessary (no doublon)
// Returns the position of insertion into the vector
// It assumes that the vector is sorted
inline unsigned HistogramClassifier::insert(const HistoPixelFeature& xj)
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
	
	HistoX.resize(numberBins);

	if (!reset)
	{
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
				histoStream>>HistoX[j].v[p];
			histoStream>>HistoX[j].c;
		}
	}
	else
	{
		unsigned garbage;
		for (unsigned j = 0; j < numberBins && histoStream.good(); ++j)
		{
			for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
				histoStream>>HistoX[j].v[p];
			histoStream>>garbage;
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
		for (unsigned j = 0; j < numberBins && histoFile.good(); ++j)
		{
			for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
				histoFile<<HistoX[j].v[p]<<" ";
			histoFile<<HistoX[j].c<<endl;
		}

	}
	else
	{
		cerr<<"Error : Could not open file "<<histogramFilename<<" for writing."<<endl;

	}
	

	histoFile.close();
}




