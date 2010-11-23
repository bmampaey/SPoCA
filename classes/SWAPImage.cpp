#include "SWAPImage.h"

const double PI = 3.14159265358979323846;
const double MIPI = 1.57079632679489661923;
const double BIPI = 6.28318530717958647692;

using namespace std;


SWAPImage::~SWAPImage()
{}


SWAPImage::SWAPImage(const string& filename)
:SunImage()
{
	readFitsImage(filename);
	if(!isSWAP(header))
		cerr<<"Error : "<<filename<<" is not SWAP!"<<endl;
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}



SWAPImage::SWAPImage(const SunImage& i)
:SunImage(i)
{	
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}


SWAPImage::SWAPImage(const SunImage* i)
:SunImage(i)
{	
	sineCorrectionParameters[0] = SWAP_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SWAP_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SWAP_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SWAP_SINE_CORR_R4 / 100.;
}


void SWAPImage::readHeader(fitsfile* fptr)
{	

	header.readKeywords(fptr);
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1");
	suncenter.y = header.get<int>("CRPIX2");
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
	observationTime = ObservationTime();

}

void SWAPImage::writeHeader(fitsfile* fptr)
{
	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x);
	header.set<int>("CRPIX2", suncenter.y);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("RSUN_ARC", radius*cdelt1);
	header.set<double>("EXPTIME", exposureTime);
	header.set<double>("HGLT_OBS", (b0 * 180)/PI);
	header.writeKeywords(fptr);
}


bool isSWAP(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SWAP") != string::npos;	
}

