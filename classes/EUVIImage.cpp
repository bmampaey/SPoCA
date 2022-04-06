#include "EUVIImage.h"
#include "colortables.h"

using namespace std;



EUVIImage::~EUVIImage()
{}


EUVIImage::EUVIImage()
:EUVImage()
{}

EUVIImage::EUVIImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(header, xAxes, yAxes)
{}

EUVIImage::EUVIImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:EUVImage(wcs, xAxes, yAxes)
{}


EUVIImage::EUVIImage(const EUVImage& i)
:EUVImage(i)
{}


EUVIImage::EUVIImage(const EUVImage* i)
:EUVImage(i)
{}

vector<Real> EUVIImage::getALCParameters()
{
	Real parameters[] = EUV_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	for(unsigned p = 0; p < temp.size(); ++p)
		temp[p] /= 100.;
	return temp;
}

void EUVIImage::parseHeader()
{

	wavelength = header.get<Real>("WAVELNTH");
	
	if (header.has("DATAMEDN"))
		median = header.get<Real>("DATAMEDN");
	if (header.has("DATAP01"))
		datap01 = header.get<EUVPixelType>("DATAP01");
	if (header.has("DATAP99"))
		datap99 = header.get<EUVPixelType>("DATAP99");
	
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
		wcs.setDistanceSunObs(DISTANCE_EARTH_SUN);
	
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
	if (header.has("RSUN"))
		wcs.setSunradius(header.get<Real>("RSUN")/wcs.cdelt1);
	else
	{
		cerr<<"Error: No sun radius found in header"<<endl;
		exit(EXIT_FAILURE);
	}

}

void EUVIImage::fillHeader()
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
	header.set<string>("DATE_OBS", wcs.date_obs);
	header.set<Real>("RSUN", wcs.sun_radius*wcs.cdelt1);
	
	if (header.has("CROTA2"))
	{
		header.set("CROTA2", wcs.getCrota2());
	}
	if (header.has("SC_ROLL"))
	{
		header.set("SC_ROLL", wcs.getCrota2());
	}
}

string EUVIImage::Instrument() const
{
	return "EUVI";
}

vector<unsigned char> EUVIImage::color_table() const
{
	switch (int(wavelength))
	{
		case 171:
		{
			unsigned char colorTable[][3] = CT_EUVI_171;
			return vector<unsigned char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		case 195:
		{
			unsigned char colorTable[][3] = CT_EUVI_195;
			return vector<unsigned char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		case 284:
		{
			unsigned char colorTable[][3] = CT_EUVI_284;
			return vector<unsigned char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}

		case 304:
		{
			unsigned char colorTable[][3] = CT_EUVI_304;
			return vector<unsigned char> (colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
			break;
		}
		
		default:
			cerr<<"Unknow wavelength "<<wavelength<<" for EUVI"<<endl;
			return vector<unsigned char>(0);
		break;
	}
}



bool isEUVI(const Header& header)
{
	return header.has("DETECTOR") && header.get<string>("DETECTOR").find("EUVI") != string::npos;
}
