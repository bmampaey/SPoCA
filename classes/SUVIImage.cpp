#include "SUVIImage.h"
#include "colortables.h"

using namespace std;

SUVIImage::~SUVIImage()
{}


SUVIImage::SUVIImage()
:EUVImage()
{}

SUVIImage::SUVIImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(header, xAxes, yAxes)
{}

SUVIImage::SUVIImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(wcs, xAxes, yAxes)
{}


SUVIImage::SUVIImage(const EUVImage& i)
:EUVImage(i)
{}


SUVIImage::SUVIImage(const EUVImage* i)
:EUVImage(i)
{}

vector<Real> SUVIImage::getALCParameters()
{
	Real parameters[] = EUV_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	for(unsigned p = 0; p < temp.size(); ++p)
		temp[p] /= 100.;
	return temp;
}

void SUVIImage::parseHeader()
{
	wavelength = header.get<Real>("WAVELNTH");
	
	if (header.has("EXPTIME"))
		exposureTime = header.get<Real>("EXPTIME");
	else
		exposureTime = 1;
	
	// We parse the header to extract the wcs coordinate system
	if(header.has("CRPIX1") and header.has("CRPIX2"))
		wcs.setSunCenter(header.get<Real>("CRPIX1") - 1, header.get<Real>("CRPIX2") - 1);
	else
		cerr<<"Error: Fits header not conform: No CRPIX1 or CRPIX2 keyword."<<endl;
	
	if (header.has("DATE-OBS"))
		wcs.setDateObs(header.get<string>("DATE-OBS"));
	
	if(header.has("CTYPE1") and header.has("CTYPE2"))
		wcs.setCType(header.get<string>("CTYPE1"), header.get<string>("CTYPE2"));
	else
		cerr<<"Error: Fits header not conform: No CTYPE1 or CTYPE2 keyword."<<endl;
	
	if(header.has("CDELT1") and header.has("CDELT2"))
		wcs.setCDelt(header.get<Real>("CDELT1"), header.get<Real>("CDELT2"));
	else
		cerr<<"Error: Fits header not conform: No CDELT1 or CDELT2 keyword."<<endl;
	
	if (header.has("HGLT_OBS"))
		wcs.setB0(header.get<Real>("HGLT_OBS"));
	else if (wcs.time_obs != 0)
		wcs.setB0(earth_latitude(wcs.time_obs));
	
	if (header.has("HGLN_OBS"))
		wcs.setL0(header.get<Real>("HGLN_OBS"));
	else
		wcs.setL0(0);
	
	if (header.has("DSUN_OBS"))
		wcs.setDistanceSunObs(header.get<double>("DSUN_OBS")/1000000.);
	else if (wcs.time_obs != 0)
		wcs.setDistanceSunObs(distance_sun_earth(wcs.time_obs));
	
	if (header.has("PC1_1") and header.has("PC1_2") and header.has("PC2_1") and header.has("PC2_2"))
	{
		wcs.setPC(header.get<Real>("PC1_1"), header.get<Real>("PC1_2"), header.get<Real>("PC2_1"), header.get<Real>("PC2_2"));
	}
	
	
	// We read the radius
	if(header.has("RSUN_OBS"))
	{
		wcs.setSunradius(header.get<Real>("RSUN_OBS"));
	}
	else
	{
		cerr<<"Error: No sun radius found in header"<<endl;
		exit(EXIT_FAILURE);
	}
	
	if(header.has("RSUN_REF"))
	{
		wcs.setSunradiusMm(header.get<Real>("RSUN_REF")/1000000.);
	}

}

void SUVIImage::fillHeader()
{
	header.set<Real>("WAVELNTH", wavelength);
	header.set<Real>("EXPTIME", exposureTime);
	header.set<Real>("CRPIX1", wcs.sun_center.x + 1);
	header.set<Real>("CRPIX2", wcs.sun_center.y + 1);
	header.set<string>("CTYPE1", wcs.ctype1);
	header.set<string>("CTYPE2", wcs.ctype2);
	header.set<Real>("CDELT1", wcs.cdelt1);
	header.set<Real>("CDELT2", wcs.cdelt2);
	header.set<string>("CUNIT1", wcs.cunit1);
	header.set<string>("CUNIT2", wcs.cunit2);
	header.set<Real>("HGLT_OBS", wcs.b0 * RADIAN2DEGREE);
	header.set<Real>("HGLN_OBS", wcs.l0 * RADIAN2DEGREE);
	header.set<Real>("CRLN_OBS", wcs.carrington_l0 * RADIAN2DEGREE);
	header.set<Real>("DSUN_OBS", wcs.dsun_obs*1000000.);
	header.set<Real>("CD1_1", wcs.cd[0][0]);
	header.set<Real>("CD1_2", wcs.cd[0][1]);
	header.set<Real>("CD2_1", wcs.cd[1][0]);
	header.set<Real>("CD2_2", wcs.cd[1][1]);
	header.set<string>("DATE-OBS",  wcs.date_obs);
	header.set<Real>("RSUN_OBS", wcs.sun_radius);
	header.set<Real>("RSUN_REF", wcs.sunradius_Mm * 1000000.);
}

string SUVIImage::Instrument() const
{
	return "SUVI";
}

void SUVIImage::enhance_contrast()
{
	// TODO
	EUVImage::enhance_contrast();
}


vector<unsigned char> SUVIImage::color_table() const
{
	// TODO
	return vector<unsigned char>(0);
}

bool isSUVI(const Header& header)
{
	return header.has("INSTRUME") && header.get<string>("INSTRUME").find("GOES-R") != string::npos;
}
