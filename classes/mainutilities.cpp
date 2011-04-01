#include "mainutilities.h"

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

inline bool readbinSize(RealFeature& binSize, const string& sbinSize)
{
	if(!sbinSize.empty())
	{
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
				string filename = outputFileName + "recentered.";
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


inline bool fileExists(const string& filename)
{
	struct stat fileinfo;
  	// Attempt to get the file attributes
	if(!stat(filename.c_str(),&fileinfo))
		return true;
	else
		return false;
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
