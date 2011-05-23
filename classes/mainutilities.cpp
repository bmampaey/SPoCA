#include "mainutilities.h"
#include <algorithm>

using namespace std;


inline unsigned readCentersFromFile(vector<RealFeature>& B, RealFeature& wavelengths, const string& centersFileName)
{
	B.clear();
	ifstream centersFile(centersFileName.c_str());
	if (centersFile.good())
	{
		centersFile>>wavelengths;
		RealFeature Bi;
		centersFile>>Bi;
		while(centersFile.good())
		{
			B.push_back(Bi);
			centersFile>>Bi;
		}
		centersFile.close();
		
	}
	else
	{
		cerr<<"Error : could not read centers from file "<<centersFileName<<endl;
	}
	return B.size();
}
/*
inline unsigned readManyCentersFromFile(vector< vector<RealFeature> >& Bs, const string& centersFileName)
{
	Bs.clear();
	
	ifstream centersFile(centersFileName.c_str());
	while(centersFile.good())
	{
		string centersLine;
		getline(centersFile, centersLine);
		isstream centersString(centersLine);
		vector<RealFeature> B;
		centersString>>B;
		Bs.push_back(B);
		centersFile.close();
		
	}
	else
	{
		cerr<<"Error : could not read centers from file "<<centersFileName<<endl;
	}
	return Bs.size();
}

inline void writeManyCentersFromFile(const vector< vector<RealFeature> >& Bs, const string& centersFileName)
{
	ofstream centersFile(centersFileName.c_str(),  ios_base::trunc);
	if (centersFile.good())
	{
		for(unsigned b = 0; b < Bs.size(); ++b)
		{
			centersFile<<Bs[b]<<endl;
		}
		centersFile.close();
		
	}
	else
	{
		cerr<<"Error : could not read centers from file "<<centersFileName<<endl;
	}
}
*/
inline unsigned readManyCentersFromFile(vector< vector<RealFeature> >& Bs, const string& centersFileName)
{
	Bs.clear();
	
	ifstream centersFile(centersFileName.c_str());
	if(centersFile.good())
	{
		centersFile>>Bs;
		centersFile.close();
		
	}
	else
	{
		cerr<<"Error : could not read centers from file "<<centersFileName<<endl;
	}
	return Bs.size();
}

inline void writeManyCentersToFile(const vector< vector<RealFeature> >& Bs, const string& centersFileName)
{
	ofstream centersFile(centersFileName.c_str(),  ios_base::trunc);
	if (centersFile.good())
	{
		centersFile<<Bs;
		centersFile.close();
	}
	else
	{
		cerr<<"Error : could not write centers to file "<<centersFileName<<endl;
	}
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
		Coordinate sunCenter = images[0]->SunCenter();
		
		for (unsigned p = 1; p < imagesFilenames.size(); ++p)
		{
			if( sunCenter.d2(images[p]->SunCenter()) > 2 )
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
		Real etai;
		etaFile>>etai;
		while(etaFile.good())
		{
			eta.push_back(etai);
			etaFile>>etai;
		}
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
		RealFeature L;
		maxLimitsFile>>L;
		while(maxLimitsFile.good())
		{
			maxLimits.push_back(L);
			maxLimitsFile>>L;
		}
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
			if(header.get<bool>(key_name))
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
