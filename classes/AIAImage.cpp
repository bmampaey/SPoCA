#include "AIAImage.h"

const double PI = 3.14159265358979323846;
const double MIPI = 1.57079632679489661923;
const double BIPI = 6.28318530717958647692;

using namespace std;


AIAImage::~AIAImage()
{}


AIAImage::AIAImage(const string& filename)
:SunImage()
{
	readFitsImage(filename);
	if(!isAIA(header))
		cerr<<"Error : "<<filename<<" is not AIA!"<<endl;
}

AIAImage::AIAImage(const SunImage& i)
:SunImage(i)
{}


AIAImage::AIAImage(const SunImage* i)
:SunImage(i)
{}



void AIAImage::readHeader(fitsfile* fptr)
{

	header.readKeywords(fptr);
	wavelength = header.get<double>("WAVELNTH");
	suncenter.x = header.get<int>("CRPIX1");
	suncenter.y = header.get<int>("CRPIX2");
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	exposureTime = header.get<double>("EXPTIME");
	b0 = (header.get<double>("CRLT_OBS")/180.)*PI; //Need to be verified
	
	median = header.get<double>("DATAMEDN");
	datap01 = header.get<PixelType>("DATAP01");
	datap95 = header.get<PixelType>("DATAP95");
	
	// We read the radius
	if(header.get<bool>("R_SUN"))
		radius = header.get<double>("R_SUN");
	else
	{
		//HACK for bad AIA fits header when R_SUN is not set
		radius = header.get<double>("RSUN_OBS");
		// RSUN_OBS is expressed in arc/sec
		radius/=cdelt1;
	}
	
	date_obs = header.get<string>("T_OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = ObservationTime();
	
}

void AIAImage::writeHeader(fitsfile* fptr)
{

	header.set<double>("WAVELNTH", wavelength);
	header.set<int>("CRPIX1", suncenter.x);
	header.set<int>("CRPIX2", suncenter.y);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("T_OBS", date_obs);
	header.set<double>("R_SUN", radius);
	header.set<double>("EXPTIME", exposureTime);
	header.set<double>("DATAMEDN", median);
	header.set<PixelType>("DATAP01",datap01);
	header.set<PixelType>("DATAP95", datap95);
	header.set<double>("CRLT_OBS", (b0 * 180)/PI);
	header.writeKeywords(fptr);
}



inline Real AIAImage::percentCorrection(const Real r)const
{

	const Real r1 = AIA_SINE_CORR_R1 / 100.;
	const Real r2 = AIA_SINE_CORR_R2 / 100.;
	const Real r3 = AIA_SINE_CORR_R3 / 100.;
	const Real r4 = AIA_SINE_CORR_R4 / 100.;
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

bool isAIA(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("AIA") != string::npos;	
}

