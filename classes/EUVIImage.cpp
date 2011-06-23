#include "EUVIImage.h"

using namespace std;


EUVIImage::~EUVIImage()
{}


EUVIImage::EUVIImage()
:EUVImage()
{
	Real parameters[] = EUVI_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	setALCParameters(temp);
}



EUVIImage::EUVIImage(const EUVImage& i)
:EUVImage(i)
{	
	Real parameters[] = EUVI_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	setALCParameters(temp);
}


EUVIImage::EUVIImage(const EUVImage* i)
:EUVImage(i)
{	
	Real parameters[] = EUVI_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	setALCParameters(temp);
}


void EUVIImage::postRead()
{
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");
	b0 = (header.get<double>("HGLT_OBS")/180.)*PI; //Need to be verified

	datap01 = header.get<EUVPixelType>("DATAP01");
	datap95 = header.get<EUVPixelType>("DATAP95");
	
	//We read the radius
	radius = header.get<double>("RSUN");
	// EUVI express the radius in arc/sec
	radius/=cdelt1;
	
	//We read the date
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = iso2ctime(date_obs);
	
}

void EUVIImage::preWrite()
{
	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("RSUN", radius*cdelt1);
	header.set<double>("EXPTIME", exposureTime);
	header.set<EUVPixelType>("DATAP01",datap01);
	header.set<EUVPixelType>("DATAP95", datap95);
	header.set<double>("HGLT_OBS", (b0 * 180)/PI);
	
}


bool isEUVI(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SECCHI") != string::npos;	
}


