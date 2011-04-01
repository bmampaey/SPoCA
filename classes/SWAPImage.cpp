#include "SWAPImage.h"

using namespace std;


SWAPImage::~SWAPImage()
{}

SWAPImage::SWAPImage()
:EUVImage()
{	
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}

SWAPImage::SWAPImage(const EUVImage& i)
:EUVImage(i)
{	
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}


SWAPImage::SWAPImage(const EUVImage* i)
:EUVImage(i)
{	
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}


void SWAPImage::postRead()
{
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");
	b0 = (header.get<double>("HGLT_OBS")/180.)*PI; //Need to be verified
	
	//We read the radius
	radius = header.get<double>("RSUN_ARC");
	// PROBA2 express the radius in arc/sec
	radius/=cdelt1;
	
	//We read the date
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = iso2ctime(date_obs);
}

void SWAPImage::preWrite()
{
	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("RSUN_ARC", radius*cdelt1);
	header.set<double>("EXPTIME", exposureTime);
	header.set<double>("HGLT_OBS", (b0 * 180)/PI);
}


bool isSWAP(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SWAP") != string::npos;	
}

