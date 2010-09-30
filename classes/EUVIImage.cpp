#include "EUVIImage.h"

using namespace std;


EUVIImage::~EUVIImage()
{}


EUVIImage::EUVIImage(const string& filename)
:SunImage(filename)
{
	readKeywords();
	if(!isEUVI(header))
		cerr<<"Error : "<<filename<<" is not EUVI!"<<endl;
}



EUVIImage::EUVIImage(const SunImage& i)
:SunImage(i)
{}


EUVIImage::EUVIImage(const SunImage* i)
:SunImage(i)
{}


void EUVIImage::readKeywords()
{
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1");
	suncenter.y = header.get<int>("CRPIX2");
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");

	datap01 = header.get<PixelType>("DATAP01");
	datap95 = header.get<PixelType>("DATAP95");
	
	//We read the radius
	radius = header.get<double>("RSUN");
	// EUVI express the radius in arc/sec
	radius/=cdelt1;
	
	//We read the date
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = ObservationTime();
	
}

void EUVIImage::writeKeywords()
{

}

inline Real EUVIImage::percentCorrection(const Real r)const
{

	const Real r1 = EUVI_SINE_CORR_R1 / 100.;
	const Real r2 = EUVI_SINE_CORR_R2 / 100.;
	const Real r3 = EUVI_SINE_CORR_R3 / 100.;
	const Real r4 = EUVI_SINE_CORR_R4 / 100.;
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

bool isEUVI(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SECCHI") != string::npos;	
}


