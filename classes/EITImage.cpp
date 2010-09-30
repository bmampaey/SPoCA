#include "EITImage.h"

using namespace std;


EITImage::~EITImage()
{}


EITImage::EITImage(const string& filename)
:SunImage(filename)
{

	readKeywords();
	if(!isEIT(header))
		cerr<<"Error : "<<filename<<" is not EIT!"<<endl;
	//In EIT images, bad pixels have negative values according to Veronique
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue_;
		}
	}

}



EITImage::EITImage(const SunImage& i)
:SunImage(i)
{}


EITImage::EITImage(const SunImage* i)
:SunImage(i)
{}


void EITImage::readKeywords()
{
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1");
	suncenter.y = header.get<int>("CRPIX2");
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");
	
	// We read the radius
	radius = header.get<double>("SOLAR_R");
	
	date_obs = header.get<string>("DATE-OBS");
	if(date_obs.empty())
	{
		date_obs = header.get<string>("DATE_OBS");
	}
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = ObservationTime();
	
	
}

void EITImage::writeKeywords()
{
}


inline Real EITImage::percentCorrection(const Real r)const
{

	const Real r1 = EIT_SINE_CORR_R1 / 100.;
	const Real r2 = EIT_SINE_CORR_R2 / 100.;
	const Real r3 = EIT_SINE_CORR_R3 / 100.;
	const Real r4 = EIT_SINE_CORR_R4 / 100.;
	if (r <= r1 || r >= r4)
		return 0;
	else if (r >= r2 && r <= r3)
		return 1;
	else if (r <= r2)
	{
		Real T = - 2*(r1-r2);
		Real phi = MIPI*(r1+r2)/(r1-r2);
		return (sin((BIPI/T)*r + phi) + 1)/2;
	}
	else // (r => r3)
	{
		Real T = 2*(r3-r4);
		Real phi = - MIPI*(r3+r4)/(r3-r4);
		return (sin((BIPI/T)*r + phi) + 1)/2;
	}

}

bool isEIT(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("EIT") != string::npos;	
}

