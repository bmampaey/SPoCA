#include "HMIImage.h"

using namespace std;

HMIImage::~HMIImage()
{}


HMIImage::HMIImage()
:EUVImage()
{}

HMIImage::HMIImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(header, xAxes, yAxes)
{}

HMIImage::HMIImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(wcs, xAxes, yAxes)
{}


HMIImage::HMIImage(const EUVImage& i)
:EUVImage(i)
{}


HMIImage::HMIImage(const EUVImage* i)
:EUVImage(i)
{}

vector<Real> HMIImage::getALCParameters()
{
	Real parameters[] = {0,0,0,0};
	return vector<Real> (parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
}


void HMIImage::parseHeader()
{
	// For HMI IC the wavelength is 6173 (according to L. Dolla)
	wavelength = 6173;
	
	if (header.has("DATAMEDN"))
		median = header.get<Real>("DATAMEDN");
	if (header.has("DATAP01"))
		datap01 = header.get<EUVPixelType>("DATAP01");
	if (header.has("DATAP95"))
		datap95 = header.get<EUVPixelType>("DATAP95");
	
	if (header.has("EXPTIME"))
		exposureTime = header.get<Real>("EXPTIME");
	else
		exposureTime = 1;
	
	// We parse the header to extract the wcs coordinate system
	if(header.has("CRPIX1") and header.has("CRPIX2"))
		wcs.setSunCenter(header.get<Real>("CRPIX1") - 1, header.get<Real>("CRPIX2") - 1);
	else
		cerr<<"Error: Fits header not conform: No CRPIX1 or CRPIX2 keyword."<<endl;
	
	if (header.has("DATE_OBS"))
		wcs.setDateObs(header.get<string>("DATE_OBS"));
	else if (header.has("DATE-OBS"))
		wcs.setDateObs(header.get<string>("DATE-OBS"));
	
	if(header.has("CDELT1") and header.has("CDELT2"))
		wcs.setCDelt(header.get<Real>("CDELT1"), header.get<Real>("CDELT2"));
	else
		cerr<<"Error: Fits header not conform: No CDELT1 or CDELT2 keyword."<<endl;
	
	if (header.has("HGLT_OBS"))
		wcs.setB0(header.get<Real>("HGLT_OBS"));
	else if (header.has("CRLT_OBS"))
		wcs.setB0(header.get<Real>("CRLT_OBS"));
	else if(header.has("SOLAR_B0"))
		wcs.setB0(header.get<Real>("SOLAR_B0"));
	else if (wcs.time_obs != 0)
		wcs.setB0(earth_latitude(wcs.time_obs));
	
	if (header.has("HGLN_OBS"))
		wcs.setL0(header.get<Real>("HGLN_OBS"));
	else
		wcs.setL0(0);
	
	if (header.has("CRLN_OBS"))
		wcs.setCarringtonL0(header.get<Real>("CRLN_OBS"));
	
	if (header.has("DSUN_OBS"))
		wcs.setDistanceSunObs(header.get<double>("DSUN_OBS")/1000000.);
	else if (wcs.time_obs != 0)
		wcs.setDistanceSunObs(distance_sun_earth(wcs.time_obs));
	
	if (header.has("CD1_1") and header.has("CD1_2") and header.has("CD2_1") and header.has("CD2_2"))
	{
		wcs.setCD(header.get<Real>("CD1_1"), header.get<Real>("CD1_2"), header.get<Real>("CD2_1"), header.get<Real>("CD2_2"));
	}
	else if (header.has("PC1_1") and header.has("PC1_2") and header.has("PC2_1") and header.has("PC2_2"))
	{
		wcs.setPC(header.get<Real>("PC1_1"), header.get<Real>("PC1_2"), header.get<Real>("PC2_1"), header.get<Real>("PC2_2"));
	}
	else if (header.has("CROTA2"))
	{
		wcs.setCrota2(header.get<Real>("CROTA2"));
	}
	
	// We read the radius
	if (header.has("R_SUN"))
		wcs.setSunradius(header.get<Real>("R_SUN"));
	else if(header.has("RSUN_OBS"))
	{
		wcs.setSunradius(header.get<Real>("RSUN_OBS")/wcs.cdelt1);
	}
	else
	{
		cerr<<"Error: No sun radius found in header"<<endl;
		exit(EXIT_FAILURE);
	}

}


void HMIImage::fillHeader()
{
	header.set<Real>("EXPTIME", exposureTime);
	header.set<Real>("CRPIX1", wcs.sun_center.x + 1);
	header.set<Real>("CRPIX2", wcs.sun_center.y + 1);
	header.set<Real>("CDELT1", wcs.cdelt1);
	header.set<Real>("CDELT2", wcs.cdelt2);
	header.set<Real>("HGLT_OBS", wcs.b0 * RADIAN2DEGREE);
	header.set<Real>("HGLN_OBS", wcs.l0 * RADIAN2DEGREE);
	header.set<Real>("CRLN_OBS", wcs.carrington_l0 * RADIAN2DEGREE);
	header.set<Real>("DSUN_OBS", wcs.dsun_obs*1000000.);
	header.set<Real>("CD1_1", wcs.cd[0][0]);
	header.set<Real>("CD1_2", wcs.cd[0][1]);
	header.set<Real>("CD2_1", wcs.cd[1][0]);
	header.set<Real>("CD2_2", wcs.cd[1][1]);
	header.set<string>("DATE_OBS", wcs.date_obs);
	header.set<Real>("R_SUN", wcs.sun_radius);
}


inline string HMIImage::Channel() const
{
	return Instrument();
}

inline string HMIImage::Label() const
{
	return Instrument() + " " + ObservationDate();
}

inline string HMIImage::Instrument() const
{
	if (header.has("CONTENT"))
	{ 
		if (header.get<string>("CONTENT").find("MAGNETOGRAM")!=string::npos)
			return "HMI_MAGNETOGRAM";
		else if (header.get<string>("CONTENT").find("CONTINUUM")!=string::npos)
			return "HMI_CONTINUUM";
		else
			return "HMI_" + header.get<string>("CONTENT");
	}
	else
	{
		return "HMI";
	}
}

bool isHMI(const Header& header)
{
	return header.has("INSTRUME") && header.get<string>("INSTRUME").find("HMI") != string::npos;
}

void HMIImage::enhance_contrast()
{
	if(Instrument() == "HMI_MAGNETOGRAM")
	{
		threshold(-30, +30);
	}
}

