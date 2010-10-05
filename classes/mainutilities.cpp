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

inline vector<SunImage*> getImagesFromFiles(const string imageType, const vector<string>& imagesFilenames, bool align)
{

	vector<SunImage*> images;

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
				#if DEBUG >= 2
				string filename = outputFileName + "recentered.";
				filename +=  stripPath(imagesFilenames[p]);
				images[p]->writeFitsImage(filename);
				#endif
			}
		}
	}
	return images;
}

inline SunImage* getImageFromFile(const string imageType, const string imageFilename)
{

	SunImage* image;

	#if DEBUG >= 1
	if(imageFilename.find(".fits")==string::npos && imageFilename.find(".fts")==string::npos)
	{
		cerr<<imageFilename<<" is not a fits file! (must end in .fits or .fts)"<<endl;
	}
	#endif
		
	if (imageType == "EIT")
		image = new EITImage(imageFilename);
	else if (imageType == "EUVI")
		image = new EUVIImage(imageFilename);
	else if (imageType == "AIA")
		image = new AIAImage(imageFilename);
	else if (imageType == "SWAP")
		image = new SWAPImage(imageFilename);
	else if (imageType == "ColorMap")
		image = new ColorMap(imageFilename);
	else if (imageType == "SunImage")
		image = new SunImage(imageFilename);
	else
	{
		FitsHeader header(imageFilename);
		if(isEIT(header))
			image = new EITImage(imageFilename);
		else if (isEUVI(header))
			image = new EUVIImage(imageFilename);
		else if (isAIA(header))
			image = new AIAImage(imageFilename);
		else if (isSWAP(header))
			image = new SWAPImage(imageFilename);
		else if (isColorMap(header))
			image = new ColorMap(imageFilename);
		else
		{
			cerr<<"Error: Unknown instrument for "<<imageFilename<<endl;
			image = NULL;
		}
	}
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
