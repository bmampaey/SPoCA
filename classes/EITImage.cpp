#include "EITImage.h"
#include "colortables.h"

using namespace std;


EITImage::~EITImage()
{}


EITImage::EITImage()
:EUVImage()
{}

EITImage::EITImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(header, xAxes, yAxes)
{}

EITImage::EITImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(wcs, xAxes, yAxes)
{}


EITImage::EITImage(const EUVImage& i)
:EUVImage(i)
{}


EITImage::EITImage(const EUVImage* i)
:EUVImage(i)
{}

vector<Real> EITImage::getALCParameters()
{
	Real parameters[] = EUV_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	for(unsigned p = 0; p < temp.size(); ++p)
		temp[p] /= 100.;
	return temp;
}

void EITImage::parseHeader()
{
	//In EIT images, bad pixels have negative values according to Veronique
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullpixelvalue;
		}
	}
	
	
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
	
	if (header.has("DATE_OBS"))
		wcs.setDateObs(header.get<string>("DATE_OBS"));
	else
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
		wcs.setDistanceSunObs(distance_sun_earth(wcs.time_obs) * 0.99);
	
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
	else if (header.has("SC_ROLL"))
	{
		wcs.setCrota2(header.get<Real>("SC_ROLL"));
	}
	
	// We read the radius
	if (header.has("SOLAR_R"))
		wcs.setSunradius(header.get<Real>("SOLAR_R"));
	else
	{
		cerr<<"Error: No sun radius found in header"<<endl;
		exit(EXIT_FAILURE);
	}
}

void EITImage::fillHeader()
{
	header.set<Real>("WAVELNTH", wavelength);
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
	header.set<string>("DATE_OBS",  wcs.date_obs);
	header.set<Real>("SOLAR_R", wcs.sun_radius);

}

void EITImage::enhance_contrast()
{
	EUVImage::enhance_contrast();
	for(unsigned j = 0; j < NumberPixels(); ++j)
	{
		if(pixels[j] != nullpixelvalue && pixels[j] > 0)
			pixels[j] = sqrt(pixels[j]);
	}
}


vector<char> EITImage::color_table() const
{
	switch (int(wavelength))
	{
		case 171:
		{
			char colorTable[][3] = CT_EIT_171;
			return vector<char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		case 195:
		{
			char colorTable[][3] = CT_EIT_195;
			return vector<char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		case 284:
		{
			char colorTable[][3] = CT_EIT_284;
			return vector<char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}

		case 304:
		{
			char colorTable[][3] = CT_EIT_304;
			return vector<char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		default:
			cerr<<"Unknow wavelength "<<wavelength<<" for EIT"<<endl;
			return vector<char>(0);
		break;
	}
}

string EITImage::Instrument() const
{return "EIT";}

bool isEIT(const Header& header)
{
	return header.has("INSTRUME") && header.get<string>("INSTRUME").find("EIT") != string::npos;	
}

