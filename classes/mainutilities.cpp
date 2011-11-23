#include "mainutilities.h"
#include <algorithm>

using namespace std;


inline bool readCentersFromStream(vector<RealFeature>& B, vector<string>& channels, istream& stream)
{
	B.clear();
	while(stream.good() && isspace(char(stream.peek())))
	{
		stream.get();
	}
	if (stream.good())
		stream>>channels;
	while(stream.good() && isspace(char(stream.peek())))
	{
		stream.get();
	}
	if (stream.good())
		stream>>B;
	return stream.good();
}

inline unsigned readCentersFromFile(vector<RealFeature>& B, vector<string>& channels, const string& centersFileName)
{
	B.clear();
	ifstream centersFile(centersFileName.c_str());
	if (not readCentersFromStream(B, channels, centersFile))
	{
		cerr<<"Error : could not read class centers from file "<<centersFileName<<endl;
	}
	return B.size();
}

inline unsigned readCentersFromFile(vector< vector<RealFeature> >& Bs, vector<string>& channels, const string& centersFileName)
{
	Bs.clear();
	vector<RealFeature> B;
	ifstream centersFile(centersFileName.c_str());
	if (not readCentersFromStream(B, channels, centersFile))
	{
		cerr<<"Error : could not read class centers from file "<<centersFileName<<endl;
	}
	else
	{
		Bs.push_back(B);
		vector<string> channels2;
		while(readCentersFromStream(B, channels2, centersFile))
		{
			bool ok = true;
			if(channels2.size() != channels.size())
			{
				cerr<<"Warning : Reading class centers from file "<<centersFileName<<". Number of channels do not match, omitting centers "<<B<<endl;
				ok = false;
			}
			else
			{
				for(unsigned p = 0; p < channels.size() && ok; ++p)
				{
					if(channels2[p] != channels[p])
					{
						cerr<<"Warning : Reading class centers from file "<<centersFileName<<". Channels do not match, omitting centers "<<B<<endl;
						ok = false;
					}
				}
				
			}
			if( ok && B.size() != Bs[0].size())
			{
				cerr<<"Warning : Reading class centers from file "<<centersFileName<<". Number of classes do not match, omitting centers "<<B<<endl;
				ok = false;
			}
			if(ok)
			{
				Bs.push_back(B);
			}
		}
	}
	return Bs.size();
}

inline bool writeCentersToStream(const vector<RealFeature> & B, const vector<string>& channels, ostream& stream)
{
	if (stream.good())
		stream<<channels;
	if (stream.good())
		stream<<"\t"<<B<<endl;
	return stream.good();
}

inline void writeCentersToFile(const vector<RealFeature> & B, const vector<string>& channels, const string& centersFileName)
{
	ofstream centersFile(centersFileName.c_str(), ios_base::trunc);
	if (not writeCentersToStream(B, channels, centersFile))
	{
		cerr<<"Error : could not write class centers to file "<<centersFileName<<endl;
	}
	centersFile.close();
}


inline void writeCentersToFile(const vector< vector<RealFeature> >& Bs, const vector<string>& channels, const string& centersFileName)
{
	ofstream centersFile(centersFileName.c_str(), ios_base::trunc);
	for(unsigned b = 0; b < Bs.size() && centersFile.good(); ++b)
	{
		if (not writeCentersToStream(Bs[b], channels, centersFile))
		{
			cerr<<"Error : could not write class centers to file "<<centersFileName<<endl;
		}
	}
	centersFile.close();
}

inline bool readbinSize(RealFeature& binSize, string sbinSize)
{
	if(!sbinSize.empty())
	{
		sbinSize.erase(remove_if(sbinSize.begin(), sbinSize.end(), ::isspace), sbinSize.end());
		if (sbinSize[0] != '(')
			sbinSize = "(" + sbinSize + ")";
		istringstream Z(sbinSize);
		Z>>binSize;
		if(Z.fail())
		{
			cerr<<"Error reading the binSize."<<endl;
			return false;
		}
	}
	return true;
}

inline vector<EUVImage*> getImagesFromFiles(const string imageType, const vector<string>& imagesFilenames, bool align)
{

	vector<EUVImage*> images;

	// We read the files
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		images.push_back(getImageFromFile(imageType, imagesFilenames[p]));
	}
	
	
	if(align) // We align the images so they all have the same sun center
	{
		RealPixLoc sunCenter = images[0]->SunCenter();
		
		for (unsigned p = 1; p < imagesFilenames.size(); ++p)
		{
			if(distance_squared(sunCenter, images[p]->SunCenter()) > 2)
			{
				cerr<<"Warning : Image "<<imagesFilenames[p]<<" will be recentered to have the same sun centre than image "<<imagesFilenames[0]<<endl;
				images[p]->recenter(sunCenter);
				#if DEBUG >= 3
				string filename = filenamePrefix + "recentered.";
				filename +=  stripPath(imagesFilenames[p]);
				images[p]->writeFits(filename);
				#endif
			}
		}
	}
	return images;
}

inline EUVImage* getImageFromFile(const string imageType, const string imageFilename)
{

	EUVImage* image;

	#if DEBUG >= 1
	if(imageFilename.find(".fits")==string::npos && imageFilename.find(".fts")==string::npos)
	{
		cerr<<imageFilename<<" is not a fits file! (must end in .fits or .fts)"<<endl;
	}
	#endif
		
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

inline ColorMap* getImageFromFile(const string imageFilename)
{

	#if DEBUG >= 1
	if(imageFilename.find(".fits")==string::npos && imageFilename.find(".fts")==string::npos)
	{
		cerr<<imageFilename<<" is not a fits file! (must end in .fits or .fts)"<<endl;
	}
	#endif
	ColorMap* image = new ColorMap();
	FitsFile file(imageFilename);
	image->readFits(file);
	return image;

}

inline unsigned readEtaFromFile (vector<Real>& eta, const string& etaFileName)
{
	eta.clear();
	ifstream etaFile(etaFileName.c_str());
	if (etaFile.good())
	{
		etaFile>>eta;
		etaFile.close();
	}
	else
	{
		cerr<<"Error : could not read eta from file "<<etaFileName<<endl;
	}
	return eta.size();
}

inline unsigned readMaxLimitsFromFile (vector<RealFeature>& maxLimits, const string& maxLimitsFileName)
{
	maxLimits.clear();
	ifstream maxLimitsFile(maxLimitsFileName.c_str());
	if (maxLimitsFile.good())
	{
		maxLimitsFile>>maxLimits;
		maxLimitsFile.close();
	}
	else
	{
		cerr<<"Error : could not read the max limits from file "<<maxLimitsFileName<<endl;
	}
	return maxLimits.size();

}

string expand(string text, const Header& header)
{
	size_t key_start = text.find_first_of('{');
	while (key_start != string::npos)
	{
		size_t key_end = text.find_first_of('}', key_start);
		if(key_end != string::npos)
		{
			string key_name = text.substr(key_start + 1, key_end - key_start - 1);
			string value = "";
			if(header.has(key_name))
			{
				value = header.get<string>(key_name);
			}
			else
			{
				cerr<<"Warning: key_name "<<key_name<<" requested in "<<text<<" not found in header."<<endl;
			}
			text.replace(key_start, key_end - key_start + 1, value);
		}
		else
		{
			cerr<<"Warning: malformed string, no closing } after position "<<key_start<<" in "<<text<<endl;
			break;
		}
		key_start = text.find_first_of('{');
	}
	return text;
}

vector<RealFeature> median_classcenters(const vector< vector<RealFeature> >& Bs)
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
		cerr<<"Warning : computing median of an empty vector."<<endl;
	}
	return Bmedian;
}
