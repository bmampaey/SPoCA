#include "EITImage.h"

using namespace std;


EITImage::~EITImage()
{}


EITImage::EITImage()
:EUVImage()
{
	sineCorrectionParameters[0] = EIT_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = EIT_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = EIT_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = EIT_SINE_CORR_R4 / 100.;
}



EITImage::EITImage(const EUVImage& i)
:EUVImage(i)
{
	sineCorrectionParameters[0] = EIT_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = EIT_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = EIT_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = EIT_SINE_CORR_R4 / 100.;
}


EITImage::EITImage(const EUVImage* i)
:EUVImage(i)
{
	sineCorrectionParameters[0] = EIT_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = EIT_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = EIT_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = EIT_SINE_CORR_R4 / 100.;
}


void EITImage::postRead()
{
	//In EIT images, bad pixels have negative values according to Veronique
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue_;
		}
	}
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");
	b0 = (header.get<double>("SOLAR_B0")/180.)*PI;
	
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
	observationTime = iso2ctime(date_obs);
	
	
}

void EITImage::preWrite()
{
	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("SOLAR_R", radius);
	header.set<double>("EXPTIME", exposureTime);
	header.set<double>("SOLAR_B0", (b0 * 180)/PI);
	

}



bool isEIT(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("EIT") != string::npos;	
}

