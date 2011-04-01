#include "HMIImage.h"

using namespace std;

HMIImage::~HMIImage()
{}


HMIImage::HMIImage()
:EUVImage()
{

}

HMIImage::HMIImage(const EUVImage& i)
:EUVImage(i)
{

}


HMIImage::HMIImage(const EUVImage* i)
:EUVImage(i)
{

}



void HMIImage::postRead()
{
	wavelength = 0;
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	b0 = (header.get<double>("CRLT_OBS")/180.)*PI; //Need to be verified
	
	median = header.get<double>("DATAMEDN");
	
	// We read the radius
	if(header.get<bool>("R_SUN"))
		radius = header.get<double>("R_SUN");
	else
	{
		//HACK for HMI fits header when R_SUN is not set
		radius = header.get<double>("RSUN_OBS");
		// RSUN_OBS is expressed in arc/sec
		radius/=cdelt1;
	}
	
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = iso2ctime(date_obs);
}


void HMIImage::preWrite()
{
	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("T_OBS", date_obs);
	header.set<double>("R_SUN", radius);
	header.set<double>("DATAMEDN", median);
	header.set<double>("CRLT_OBS", (b0 * 180)/PI);
}


bool isHMI(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("HMI") != string::npos;	
}


