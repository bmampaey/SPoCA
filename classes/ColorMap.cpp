#include "ColorMap.h"

using namespace std;


ColorMap::~ColorMap()
{}


ColorMap::ColorMap(const string& filename)
:SunImage(filename)
{
	nullvalue_ = 0;
}

ColorMap::ColorMap(const long xAxes, const long yAxes)
:SunImage(xAxes, yAxes)
{
	nullvalue_ = 0;
}


ColorMap::ColorMap(const SunImage& i)
:SunImage(i)
{
	nullvalue_ = 0;
}


ColorMap::ColorMap(const SunImage* i)
:SunImage(i)
{
	nullvalue_ = 0;
}


void ColorMap::readHeader(fitsfile* fptr)
{

	header.readKeywords(fptr);
	wavelength = 0;
	suncenter.x = header.get<int>("CRPIX1");
	suncenter.y = header.get<int>("CRPIX2");
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = 0;
	
	// We read the radius
	radius = header.get<double>("RADIUS");
	
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = ObservationTime();
	
	nullvalue_ = 0;
	
	
}

void ColorMap::writeHeader(fitsfile* fptr)
{

	header.set<string>("INSTRUME", "SPoCA");
	header.set<double>("RADIUS", radius);
	header.set<int>("CRPIX1", suncenter.x);
	header.set<int>("CRPIX2", suncenter.y);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.writeKeywords(fptr);

}


bool isColorMap(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SPoCA") != string::npos;	
}

