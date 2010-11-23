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
	sineCorrectionParameters[0] = AIA_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = AIA_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = AIA_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = AIA_SINE_CORR_R4 / 100.;
}

AIAImage::AIAImage(const SunImage& i)
:SunImage(i)
{
	sineCorrectionParameters[0] = AIA_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = AIA_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = AIA_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = AIA_SINE_CORR_R4 / 100.;
}


AIAImage::AIAImage(const SunImage* i)
:SunImage(i)
{
	sineCorrectionParameters[0] = AIA_SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = AIA_SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = AIA_SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = AIA_SINE_CORR_R4 / 100.;
}



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


bool isAIA(const FitsHeader& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("AIA") != string::npos;	
}

