#include "ColorMap.h"

const double PI = 3.14159265358979323846;
const double MIPI = 1.57079632679489661923;
const double BIPI = 6.28318530717958647692;

using namespace std;


ColorMap::~ColorMap()
{}


ColorMap::ColorMap(const string& filename)
:SunImage()
{
	readFitsImage(filename);
	if(!isColorMap(header))
		cerr<<"Error : "<<filename<<" is not a SPoCA ColorMap!"<<endl;
	
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
	
	
	b0 = (header.get<double>("SOLAR_B0")/180.)*PI;
	
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
	header.set<double>("SOLAR_B0", (b0 * 180)/PI);
	header.writeKeywords(fptr);

}


bool isColorMap(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SPoCA") != string::npos;	
}

