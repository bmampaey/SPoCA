#include "mainutilities.h"
#include <algorithm>

using namespace std;

inline bool read_more(ifstream& file)
{
	while(file.good() && isspace(file.peek()))
		file.get();
	return file.good();
}

bool readCentersFromFile(const string& filename, vector<string>& channels, vector<RealFeature>& B)
{
	B.clear();
	vector<string> read_channels;
	ifstream file(filename.c_str());
	if(file)
	{
		file>>read_channels>>B;
		if(! file)
		{
			cerr<<"Error : could not read class centers from file "<<filename<<endl;
			return false;
		}
		else if(channels.size() == 0)
		{
			channels = read_channels;
		}
		else if(channels.size() != read_channels.size())
		{
			cerr<<"Error : number of channels mismatch in class centers file "<<filename<<": expected "<<channels.size()<<" channels but read only "<<read_channels.size()<<endl;
			return false;
		}
		else
		{
			for(unsigned p = 0; p < channels.size(); ++p)
			{
				if(channels[p] != read_channels[p])
				{
					cerr<<"Error : channels mismatch in class centers file "<<filename<<": expected channel "<<channels[p]<<" but read "<<read_channels[p]<<endl;
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

bool readCentersFromFile(const string& filename, vector<string>& channels, deque< vector<RealFeature> >& Bs, const unsigned max)
{
	Bs.clear();
	if(max == 0)
		return true;
	
	vector<RealFeature> read_B;
	vector<string> read_channels;
	ifstream file(filename.c_str());
	if(file)
	{
		for(unsigned i = 0; i < max && read_more(file); ++i)
		{
			file>>read_channels>>read_B;
			if(! file)
			{
				cerr<<"Error : could not read class centers from file "<<filename<<endl;
				return false;
			}
			else if(channels.size() == 0)
			{
				channels = read_channels;
			}
			else if(channels.size() != read_channels.size())
			{
				cerr<<"Error : number of channels mismatch in class centers file "<<filename<<": expected "<<channels.size()<<" channels but read only "<<read_channels.size()<<endl;
				return false;
			}
			else
			{
				for(unsigned p = 0; p < channels.size(); ++p)
				{
					if(channels[p] != read_channels[p])
					{
						cerr<<"Error : channels mismatch in class centers file "<<filename<<": expected channel "<<channels[p]<<" but read "<<read_channels[p]<<endl;
						return false;
					}
				}
			}
			Bs.push_back(read_B);
		}
		return true;
	}
	return false;
}

bool writeCentersToFile(const string& filename, const vector<string>& channels, const vector<RealFeature>& B)
{
	ofstream file(filename.c_str(), ios_base::trunc);
	if(file)
	{
		file<<setiosflags(ios::fixed)<<channels<<"\t"<<B<<endl;
		if(! file)
		{
			cerr<<"Error : could not write class centers to file "<<filename<<endl;
			return false;
		}
		return true;
	}
	return false;
}

bool writeCentersToFile(const string& filename, const vector<string>& channels, const deque< vector<RealFeature> >& Bs, const unsigned max)
{
	ofstream file(filename.c_str(), ios_base::trunc);
	if(file)
	{
		for(unsigned i = 0; i < max+1 && file.good() && i < Bs.size(); ++i)
		{
			file<<setiosflags(ios::fixed)<<channels<<"\t"<<Bs[i]<<endl;
			if(! file)
			{
				cerr<<"Error : could not write class centers to file "<<filename<<endl;
				return false;
			}
		}
		return true;
	}
	return false;
}

bool readCentersEtasFromFile(const string& filename, vector<string>& channels, vector<RealFeature>& B, vector<Real>& Eta)
{
	B.clear();
	Eta.clear();
	vector<string> read_channels;
	ifstream file(filename.c_str());
	if(file)
	{
		file>>read_channels>>B>>Eta;
		if(! file)
		{
			cerr<<"Error : could not read class centers from file "<<filename<<endl;
			return false;
		}
		else if(channels.size() == 0)
		{
			channels = read_channels;
		}
		else if(channels.size() != read_channels.size())
		{
			cerr<<"Error : number of channels mismatch in class centers file "<<filename<<": expected "<<channels.size()<<" channels but read only "<<read_channels.size()<<endl;
			return false;
		}
		else
		{
			for(unsigned p = 0; p < channels.size(); ++p)
			{
				if(channels[p] != read_channels[p])
				{
					cerr<<"Error : channels mismatch in class centers file "<<filename<<": expected channel "<<channels[p]<<" but read "<<read_channels[p]<<endl;
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

bool readCentersEtasFromFile(const string& filename, vector<string>& channels, deque< vector<RealFeature> >& Bs, deque< vector<Real> >& Etas, const unsigned max)
{
	Bs.clear();
	Etas.clear();
	if(max == 0)
		return true;
	
	vector<RealFeature> read_B;
	vector<Real> read_Eta;
	vector<string> read_channels;
	ifstream file(filename.c_str());
	if(file)
	{
		for(unsigned i = 0; i < max && read_more(file); ++i)
		{
			file>>read_channels>>read_B>>read_Eta;
			if(! file)
			{
				cerr<<"Error : could not read class centers from file "<<filename<<endl;
				return false;
			}
			else if(channels.size() == 0)
			{
				channels = read_channels;
			}
			else if(channels.size() != read_channels.size())
			{
				cerr<<"Error : number of channels mismatch in class centers file "<<filename<<": expected "<<channels.size()<<" channels but read only "<<read_channels.size()<<endl;
				return false;
			}
			else
			{
				for(unsigned p = 0; p < channels.size(); ++p)
				{
					if(channels[p] != read_channels[p])
					{
						cerr<<"Error : channels mismatch in class centers file "<<filename<<": expected channel "<<channels[p]<<" but read "<<read_channels[p]<<endl;
						return false;
					}
				}
			}
			Bs.push_back(read_B);
			Etas.push_back(read_Eta);
		}
		return true;
	}
	return false;
}

bool writeCentersEtasToFile(const string& filename, const vector<string>& channels, const vector<RealFeature>& B, const vector<Real>& Eta)
{
	ofstream file(filename.c_str(), ios_base::trunc);
	if(file)
	{
		file<<setiosflags(ios::fixed)<<channels<<"\t"<<B<<"\t"<<Eta<<endl;
		if(! file)
		{
			cerr<<"Error : could not write class centers to file "<<filename<<endl;
			return false;
		}
		return true;
	}
	return false;
}

bool writeCentersEtasToFile(const string& filename, const vector<string>& channels, const deque< vector<RealFeature> >& Bs, const deque< vector<Real> >& Etas, const unsigned max)
{
	ofstream file(filename.c_str(), ios_base::trunc);
	if(file)
	{
		for(unsigned i = 0; i < max + 1 && file.good() && i < Bs.size() && i < Etas.size(); ++i)
		{
			file<<setiosflags(ios::fixed)<<channels<<"\t"<<Bs[i]<<"\t"<<Etas[i]<<endl;
			if(! file)
			{
				cerr<<"Error : could not write class centers to file "<<filename<<endl;
				return false;
			}
		}
		return true;
	}
	return false;
}

inline bool readCoordinate(RealPixLoc& coordinate, string sCoordinate)
{
	if(!sCoordinate.empty())
	{
		sCoordinate.erase(remove_if(sCoordinate.begin(), sCoordinate.end(), ::isspace), sCoordinate.end());
		if (sCoordinate[0] != '(')
			sCoordinate = "(" + sCoordinate + ")";
		istringstream Z(sCoordinate);
		Z>>coordinate;
		if(Z.fail())
		{
			cerr<<"Error reading the coordinate."<<endl;
			return false;
		}
	}
	return true;
}


inline EUVImage* getImageFromFile(const string imageType, const string imageFilename)
{
	EUVImage* image;
	
	#if defined EXTRA_SAFE
	if(imageFilename.find(".fits")==string::npos && imageFilename.find(".fts")==string::npos)
	{
		cerr<<"Warning: "<<imageFilename<<" is not a fits file! (must end in .fits or .fts)"<<endl;
	}
	#endif
	if(!isFile(imageFilename))
	{
		cerr<<"Error: Cannot find file "<<imageFilename<<endl;
		exit(EXIT_FAILURE);
	}
	FitsFile file(imageFilename);
	if (imageType == "EIT")
		image = new EITImage();
	else if (imageType == "EUVI")
		image = new EUVIImage();
	else if (imageType == "AIA")
		image = new AIAImage();
	else if (imageType == "SWAP")
		image = new SWAPImage();
	else if (imageType == "HMI")
		image = new HMIImage();
	else
	{
		Header header;
		file.readHeader(header);
		if(isEIT(header))
			image = new EITImage();
		else if (isEUVI(header))
			image = new EUVIImage();
		else if (isAIA(header))
			image = new AIAImage();
		else if (isSWAP(header))
			image = new SWAPImage();
		else if (isHMI(header))
			image = new HMIImage();
		else
		{
			cerr<<"Error: Unknown instrument for "<<imageFilename<<endl;
			image = new EUVImage();
		}
	}
	image->readFits(file);
	return image;
}

inline ColorMap* getColorMapFromFile(const string imageFilename)
{

	#if defined EXTRA_SAFE
	if(imageFilename.find(".fits")==string::npos && imageFilename.find(".fts")==string::npos)
	{
		cerr<<imageFilename<<" is not a fits file! (must end in .fits or .fts)"<<endl;
	}
	#endif
	if(!isFile(imageFilename))
	{
		cerr<<"Error: Cannot find file "<<imageFilename<<endl;
		exit(EXIT_FAILURE);
	}
	ColorMap* image = new ColorMap();
	FitsFile file(imageFilename);
	image->readFits(file);
	return image;

}


vector<RealFeature> median(const deque< vector<RealFeature> >& Bs)
{
	vector<RealFeature> Bmedian;
	if(Bs.size() > 0)
	{
		Bmedian.resize(Bs[0].size());
		for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
		{
			for(unsigned i = 0; i < Bs[0].size(); ++i)
			{
				vector<Real> values(Bs.size());
				for(unsigned b = 0; b < Bs.size(); ++b)
				{
					values[b] = Bs[b].at(i).v[p];
				}
				sort(values.begin(), values.end());
				Bmedian[i].v[p] = values[values.size()/2];
			}
		}
	}
	else
	{
		throw invalid_argument("Vector of class center is empty, cannot compute median of an empty vector.");
	}
	return Bmedian;
}


vector<Real> median(const deque< vector<Real> >& Etas)
{
	vector<Real> Etamedian;
	if(Etas.size() > 0)
	{
		Etamedian.resize(Etas[0].size());
		for(unsigned i = 0; i < Etas[0].size(); ++i)
		{
			vector<Real> values(Etas.size());
			for(unsigned b = 0; b < Etas.size(); ++b)
			{
				values[b] = Etas[b].at(i);
			}
			sort(values.begin(), values.end());
			Etamedian[i] = values[values.size()/2];
		}
	}
	else
	{
		throw invalid_argument("Vector of Eta is empty, cannot compute median of an empty vector.");
	}
	return Etamedian;
}

bool reorderImages(vector<EUVImage*>& images, const vector<string>& channels)
{
	if(channels.size() != images.size())
	{
		return false;
	}
	else
	{
		for (unsigned p = 0; p < images.size(); ++p)
		{
			if(channels[p] != images[p]->Channel())
			{
				unsigned pp = p+1;
				while(pp < images.size() && channels[p] != images[pp]->Channel())
					++pp;
				if(pp < images.size())
				{
					EUVImage* temp = images[pp];
					images[pp] = images[p];
					images[p] = temp;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return true;
}

