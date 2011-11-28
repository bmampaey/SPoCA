#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <assert.h>
#include "SunImage.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

template<class T>
SunImage<T>::~SunImage()
{
}

template<class T>
SunImage<T>::SunImage(const unsigned& xAxes, const unsigned& yAxes)
:Image<T>(xAxes, yAxes), wcs(RealPixLoc(xAxes/2., yAxes/2.), xAxes/2.)
{
}

template<class T>
SunImage<T>::SunImage(const unsigned& xAxes, const unsigned& yAxes, const RealPixLoc& sun_center, const Real& sun_radius)
:Image<T>(xAxes,yAxes), wcs(sun_center, sun_radius)
{
}

template<class T>
SunImage<T>::SunImage(const SunImage& i)
:Image<T>(i), header(i.header), wcs(i.wcs)
{
}


template<class T>
SunImage<T>::SunImage(const SunImage* i)
:Image<T>(i), header(i->header), wcs(i->wcs)
{
}

template<class T>
SunImage<T>::SunImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:Image<T>(xAxes, yAxes), header(header)
{
	parseHeader();
}

template<class T>
SunImage<T>::SunImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:Image<T>(xAxes, yAxes), wcs(wcs)
{
}


template<class T>
inline Header& SunImage<T>::getHeader()
{
	fillHeader();
	return header;
}


template<class T>
inline WCS& SunImage<T>::getWCS()
{
	return wcs;
}

template<class T>
inline RealPixLoc SunImage<T>::SunCenter() const
{
	return wcs.sun_center;
}

template<class T>
inline Real SunImage<T>::SunRadius() const
{
	return wcs.sun_radius;
}

template<class T>
inline Real SunImage<T>::PixelLength() const
{
	return wcs.cdelt1;
}

template<class T>
inline Real SunImage<T>::PixelWidth() const
{
	return wcs.cdelt2;
}

template<class T>
inline time_t SunImage<T>::ObservationTime()const
{
	return wcs.time_obs;
}

template<class T>
inline string SunImage<T>::ObservationDate() const
{
	return wcs.date_obs;
}

template<class T>
inline string SunImage<T>::Instrument() const
{
	if(header.has("INSTRUME"))
		return header.get<string>("INSTRUME");
	else
		return "UNKNOWN";
}

template<class T>
inline string SunImage<T>::Label() const
{
	return Instrument() + " " + ObservationDate();
}

template<class T>
inline void SunImage<T>::nullifyAboveRadius(const Real radiusRatio)
{
	Real radius2 = radiusRatio*radiusRatio*wcs.sun_radius*wcs.sun_radius;
	Real max_x = -wcs.sun_center.x + this->xAxes;
	Real max_y = -wcs.sun_center.y + this->yAxes;
	T* pixel_value = this->pixels;
	
	for (Real y = -wcs.sun_center.y; y < max_y; ++y)
	{
		for (Real x = -wcs.sun_center.x; x < max_x; ++x)
		{
			if (x * x + y * y > radius2)
				*pixel_value = this->nullpixelvalue;
			
			++pixel_value;
		}
	}
}

template<class T>
inline void SunImage<T>::nullifyAboveLongLat(Real longitude, Real latitude)
{
	longitude = longitude < 0 ? - longitude * DEGREE2RADIAN : longitude * DEGREE2RADIAN;
	latitude = latitude < 0 ? - latitude * DEGREE2RADIAN : latitude * DEGREE2RADIAN;


	for (unsigned y = 0; y < this->yAxes; ++y)
	{
		for (unsigned x = 0; x < this->xAxes; ++x)
		{
			HGS hgs = toHGS(RealPixLoc(x, y));
			if (!hgs)
			{
				this->pixel(x, y) = this->nullpixelvalue;
			}
			else if(abs(hgs.longitude) > longitude || abs(hgs.latitude) > latitude)
			{
				this->pixel(x, y) = this->nullpixelvalue;
			}
		}
	}
}

template<class T>
inline void SunImage<T>::recenter(const RealPixLoc& newCenter)
{
	int delta = int(wcs.sun_center.x - newCenter.x + 0.5) + (int(wcs.sun_center.y - newCenter.y + 0.5) * this->xAxes);
	if(delta < 0)
	{
		memmove(this->pixels - delta, this->pixels, (this->numberPixels + delta + 1) * sizeof(T));
		fill(this->pixels, this->pixels-delta, this->nullpixelvalue);
	}
	else if (delta > 0)
	{
		memmove(this->pixels, this->pixels + delta, (this->numberPixels - delta + 1) * sizeof(T));
		fill(this->pixels + this->numberPixels - delta, this->pixels + this->numberPixels, this->nullpixelvalue);
	}
	wcs.sun_center = newCenter;
}


template<class T>
inline void SunImage<T>::rotate(const int delta_t)
{
	// We make a copy of the original image 
	Image<T> original(this);
	
	//We compute for each pixel in the new image what is the original location
	RealPixLoc new_location;
	T* new_value = this->pixels;
	for(new_location.y = 0;  new_location.y < this->yAxes; ++new_location.y)
	{
		for(new_location.x = 0; new_location.x < this->xAxes; ++new_location.x)
		{
			RealPixLoc original_location = rotate(new_location, delta_t);
			if(!original_location)
				*new_value = this->nullpixelvalue;
			else
				*new_value = original.interpolate(original_location);
			
			++new_value;
		}
	}
}


template<class T>
inline SunImage<T>* SunImage<T>::shifted_like(const SunImage* img) const
{
	SunImage<T>* shifted_image = new SunImage<T>(img->wcs, img->xAxes, img->yAxes);
	
	//We compute for each pixel in the rotated image what is the original location
	RealPixLoc shifted_location;
	T* shifted_value = shifted_image->pixels;
	for(shifted_location.y = 0;  shifted_location.y < shifted_image->yAxes; ++shifted_location.y)
	{
		for(shifted_location.x = 0; shifted_location.x < shifted_image->xAxes; ++shifted_location.x)
		{
			RealPixLoc original_location = shifted_image->shift_like(shifted_location, this);
			if(!original_location)
				*shifted_value = shifted_image->null();
			else
				*shifted_value = Image<T>::interpolate(original_location);
			
			++shifted_value;
		}
	}
	return shifted_image;

}


template<class T>
inline void SunImage<T>::shift_like(const SunImage* img)
{
	// We make a copy of the original image 
	Image<T> original(this);
	
	//We compute for each pixel in the new image what is the original location
	RealPixLoc new_location;
	T* new_value = this->pixels;
	for(new_location.y = 0;  new_location.y < this->yAxes; ++new_location.y)
	{
		for(new_location.x = 0; new_location.x < this->xAxes; ++new_location.x)
		{
			RealPixLoc original_location = img->shift_like(new_location, this);
			if(!original_location)
				*new_value = this->nullpixelvalue;
			else
				*new_value = original.interpolate(original_location);
			
			++new_value;
		}
	}
	wcs = img->wcs;
}


template<class T>
inline RealPixLoc SunImage<T>::rotate(const RealPixLoc c, const int delta_t) const
{
	HGS hgs = toHGS(c);
	if(! hgs)
		return RealPixLoc::null();
	
	hgs.longitude += delta_t * SunDifferentialAngularSpeed(hgs.latitude);
	
	if(hgs.longitude > MIPI || hgs.longitude < -MIPI)
		return RealPixLoc::null();
	else
		return toRealPixLoc(hgs);

}


template<class T>
inline RealPixLoc SunImage<T>::shift_like(const RealPixLoc c, const SunImage* img) const
{
	int delta_t = int(difftime(img->ObservationTime(),ObservationTime()));
	HGS hgs = toHGS(c);
	if(! hgs)
		return RealPixLoc::null();
	
	hgs.longitude += delta_t * SunDifferentialAngularSpeed(hgs.latitude);
	
	if(hgs.longitude > MIPI || hgs.longitude < -MIPI)
		return RealPixLoc::null();
	else
		return img->toRealPixLoc(hgs);

}

template<class T>
inline HPC SunImage<T>::toHPC(const RealPixLoc& c) const
{
	Real rx = (c.x - wcs.sun_center.x); 
	Real ry = (c.y - wcs.sun_center.y);
	return HPC(rx * wcs.cd[0][0] + ry * wcs.cd[0][1], rx * wcs.cd[1][0] + ry * wcs.cd[1][1]);
}


template<class T>
inline HCC SunImage<T>::toHCC(const RealPixLoc& c) const
{
	return toHCC(toHPC(c));
}

template<class T>
inline HGS SunImage<T>::toHGS(const RealPixLoc& c) const
{
	return toHGS(toHCC(c));
}

template<class T>
inline RealPixLoc SunImage<T>::toRealPixLoc(const HPC& hpc) const
{
	Real rx = hpc.x * wcs.icd[0][0] + hpc.y * wcs.icd[0][1];
	Real ry = hpc.x * wcs.icd[1][0] + hpc.y * wcs.icd[1][1];
	return RealPixLoc(rx+wcs.sun_center.x, ry+wcs.sun_center.y);
}

template<class T>
inline HCC SunImage<T>::toHCC(const HPC& hpc) const
{
	double hpc_x = hpc.x * ARCSEC2RADIAN;
	double hpc_y = hpc.y * ARCSEC2RADIAN;
	double cosx = cos(hpc_x);
	double sinx = sin(hpc_x);
	double cosy = cos(hpc_y);
	double siny = sin(hpc_y);
	// We compute the dist between the sun observer and the surface of the sun
	double q =  wcs.dsun_obs * cosy * cosx;
	double dist = (q*q) -  (wcs.dsun_obs* wcs.dsun_obs) + (wcs.sunradius_Mm*wcs.sunradius_Mm);
	if (dist >= 0)
	{
		dist = q - sqrt(dist); 
		return HCC(dist * cosy * sinx, dist * siny,  wcs.dsun_obs - (dist * cosy * cosx));
	}
	else
		return HCC::null();
}


template<class T>
inline HGS SunImage<T>::toHGS(const HPC& hpc) const
{
	return toHGS(toHCC(hpc));
}




template<class T>
inline RealPixLoc SunImage<T>::toRealPixLoc(const HCC& hcc) const
{
	return toRealPixLoc(toHPC(hcc));
}

template<class T>
inline HPC SunImage<T>::toHPC(const HCC& hcc) const
{
	if(! hcc)
		return HPC::null();
	else
	{
		double zeta =  wcs.dsun_obs - hcc.z;
		double dist = sqrt(hcc.x * hcc.x + hcc.y * hcc.y + zeta * zeta);
		double rx = atan2(double(hcc.x), zeta);
		double ry = asin(double(hcc.y) / dist);
		return HPC(rx*RADIAN2ARCSEC, ry*RADIAN2ARCSEC);
	}
}

template<class T>
inline HGS SunImage<T>::toHGS(const HCC& hcc) const
{
	if(! hcc)
		return HGS::null();
	else
	{
		//! TODO It seems that the hcc coordinate returned is not exact and lies outside of the sun
		Real latitude = MIPI;
		if((hcc.y * wcs.cos_b0 + hcc.z * wcs.sin_b0)/wcs.sunradius_Mm < 1)
			latitude = asin((hcc.y * wcs.cos_b0 + hcc.z * wcs.sin_b0)/wcs.sunradius_Mm);
		Real longitude = atan2(hcc.x, hcc.z * wcs.cos_b0 - hcc.y * wcs.sin_b0) + wcs.l0;
		
		return HGS(longitude, latitude);
	}
}


template<class T>
inline HCC SunImage<T>::toHCC(const HGS& hgs) const
{
	if (! hgs)
		return HCC::null();
	else
	{
		Real cos_longitude = cos(hgs.longitude - wcs.l0);
		Real sin_longitude = sin(hgs.longitude - wcs.l0);
		Real cos_latitude = cos(hgs.latitude);
		Real sin_latitude = sin(hgs.latitude);
		return HCC(wcs.sunradius_Mm * cos_latitude * sin_longitude, wcs.sunradius_Mm * (sin_latitude * wcs.cos_b0 - cos_latitude * cos_longitude * wcs.sin_b0), wcs.sunradius_Mm * (sin_latitude * wcs.sin_b0 + cos_latitude * cos_longitude * wcs.cos_b0));
	}
}

template<class T>
inline RealPixLoc SunImage<T>::toRealPixLoc(const HGS& hgs) const
{
	return toRealPixLoc(toHCC(hgs));
}

template<class T>
inline HPC SunImage<T>::toHPC(const HGS& hgs) const
{
	return toHPC(toHCC(hgs));
}

template<class T>
inline T SunImage<T>::interpolate(const HGS& c) const
{
	return Image<T>::interpolate(toRealPixLoc(c));
}

template<class T>
inline T SunImage<T>::interpolate(const HPC& c) const
{
	return Image<T>::interpolate(toRealPixLoc(c));
}

template<class T>
inline T SunImage<T>::interpolate(const HCC& c) const
{
	return Image<T>::interpolate(toRealPixLoc(c));
}

template<class T>
inline Real SunImage<T>::PixelArea() const
{
	return wcs.cdelt1 * wcs.cdelt2;
}

template<class T>
inline Real SunImage<T>::RealPixelArea(const RealPixLoc& c) const
{
	Real rx = c.x - wcs.sun_center.x;
	Real ry = c.y - wcs.sun_center.y;
	Real sigma = (wcs.sun_radius * wcs.sun_radius) - (rx * rx) - (ry * ry);
	if(sigma > 0)
		return (wcs.sunradius_Mm * wcs.sunradius_Mm) / (sqrt(sigma) * wcs.sun_radius);
	else
		return NAN;
}

template<class T>
inline Real SunImage<T>::RawPixelArea() const
{
	return PixelArea() * ((BIPI * BIPI * wcs.dsun_obs * wcs.dsun_obs) / (360. * 3600. * 360. * 3600.));
}

template<class T>
inline vector<HGS> SunImage<T>::HGSmap() const
{
	vector<HPC> hpc = HPCmap();
	vector<HGS> map(hpc.size(), HGS::null());
	for(unsigned m = 0; m < hpc.size(); ++m)
		map[m] = toHGS(hpc[m]);
	return map;
}


template<class T>
inline vector<HPC> SunImage<T>::HPCmap() const
{
	vector<HPC> map(this->numberPixels);
	unsigned j = 0;
	for(Real y = 0; y < this->yAxes; ++y)
	{
		for(Real x = 0; x < this->xAxes; ++x)
		{
			map[j] = toHPC(RealPixLoc(x, y));
			++j;
		}
	}
	return map;
}

template<class T>
inline vector<HCC> SunImage<T>::HCCmap() const
{
	vector<HPC> hpc = HPCmap();
	vector<HCC> map(hpc.size(), HCC::null());
	for(unsigned m = 0; m < hpc.size(); ++m)
		map[m] = toHCC(hpc[m]);
	return map;
}

template<class T>
inline FitsFile& SunImage<T>::writeFits(FitsFile& file, int mode, const string imagename)
{
	fillHeader();
	Image<T>::writeFits(file, mode, imagename);
	file.writeHeader(header);
	return file;
}

template<class T>
inline FitsFile& SunImage<T>::readFits(FitsFile& file)
{
	file.readHeader(header);
	Image<T>::readFits(file);
	parseHeader();
	return file;
}

template<class T>
void SunImage<T>::parseHeader()
{

	// We parse the header to extract the wcs coordinate system
	
	if(header.has("CRPIX1") and header.has("CRPIX2"))
		wcs.setSunCenter(header.get<Real>("CRPIX1") - 1, header.get<Real>("CRPIX2") - 1);
	else
		cerr<<"Error: Fits header not conform: No CRPIX1 or CRPIX2 keyword."<<endl;
	
	if (header.has("T_OBS"))
		wcs.setDateObs(header.get<string>("T_OBS"));
	else if (header.has("DATE_OBS"))
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
		wcs.setDistanceSunObs(header.get<Real>("DSUN_OBS")/1000000.);
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
	
	// We try to guess the radius, but it is defined differently for each instrument
	if (header.has("R_SUN"))
	{
		wcs.setSunradius(header.get<Real>("R_SUN"));
	}
	else if (header.has("RSUN_OBS"))
	{
		wcs.setSunradius(header.get<Real>("RSUN_OBS")/wcs.cdelt1);
	}
	else if (header.has("RSUN_ARC"))
	{
		wcs.setSunradius(header.get<Real>("RSUN_ARC")/wcs.cdelt1);
	}
	else
	{
		cerr<<"Error: No sun radius found in header"<<endl;
	}
}

template<class T>
void SunImage<T>::fillHeader()
{
	header.set<Real>("CRPIX1", wcs.sun_center.x + 1);
	header.set<Real>("CRPIX2", wcs.sun_center.y + 1);
	header.set<Real>("CDELT1", wcs.cdelt1);
	header.set<Real>("CDELT2", wcs.cdelt2);
	header.set<string>("DATE_OBS", wcs.date_obs);
	header.set<Real>("R_SUN", wcs.sun_radius);
	header.set<Real>("HGLT_OBS", wcs.b0 * RADIAN2DEGREE);
	header.set<Real>("HGLN_OBS", wcs.l0 * RADIAN2DEGREE);
	header.set<Real>("CRLN_OBS", wcs.carrington_l0 * RADIAN2DEGREE);
	header.set<Real>("DSUN_OBS", wcs.dsun_obs*1000000.);
	header.set<Real>("CD1_1", wcs.cd[0][0]);
	header.set<Real>("CD1_2", wcs.cd[0][1]);
	header.set<Real>("CD2_1", wcs.cd[1][0]);
	header.set<Real>("CD2_2", wcs.cd[1][1]);
}

template<class T>
bool SunImage<T>::writeFits(const std::string& filename, int mode, const string imagename)
{
	FitsFile file(filename, FitsFile::overwrite);
	this->writeFits(file, mode, imagename);
	return file.isGood();
}

template<class T>
bool SunImage<T>::readFits(const std::string& filename)
{
	FitsFile file(filename);
	this->readFits(file);
	return file.isGood();
}


template<class T>
void SunImage<T>::equirectangular_projection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	T* j = this->pixels;
	
	Real dy = PI / Real(this->yAxes), dx = PI / Real(this->xAxes);
	
	if(exact)
	{
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = (py * dy) - MIPI;
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = (px * dx) - MIPI;
				*j = image->interpolate(HGS(longitude, latitude));
				++j;
			}
		}
	}
	else
	{
		Real sun_radius = image->SunRadius();
		RealPixLoc sun_center = image->SunCenter();
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = (py * dy) - MIPI;
			Real cos_lat = cos(latitude);
			Real iy = sun_center.y + (sin(latitude) * sun_radius);
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = (px * dx) - MIPI;
				Real ix = sun_center.x + (sun_radius * cos_lat * sin(longitude));
				*j = image->interpolate(ix, iy);
				++j;
			}
		}
	}
}


template<class T>
void SunImage<T>::equirectangular_deprojection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	
	Real dy = Real(image->Yaxes()) / PI, dx = Real(image->Xaxes()) / PI;
	if(exact)
	{
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			for(unsigned ix = 0; ix < this->xAxes; ++ix)
			{
				HGS hgs = this->toHGS(RealPixLoc(ix,iy));
				if(!(!hgs))
				{
					Real px = (hgs.longitude + MIPI) * dx;
					Real py = (hgs.latitude + MIPI) * dy;
					this->pixel(ix,iy) = image->interpolate(px, py);
				}
			}
		}
	}
	else
	{
		Real sun_radius = SunRadius();
		RealPixLoc sun_center = SunCenter();
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			Real ry = Real(iy - sun_center.y) / sun_radius;
			if(ry <= 1. && ry >= -1.)
			{
				Real latitude = asin(ry);
				Real cos_lat = cos(latitude);
				for(unsigned ix = 0; ix < this->xAxes; ++ix)
				{
					Real rx = Real(ix - sun_center.x) / sun_radius;
					if(rx*rx + ry*ry <= 1)
					{
						Real longitude = asin(rx / cos_lat);
						Real px = (longitude + MIPI) * dx;
						Real py = (latitude + MIPI) * dy;
						this->pixel(ix, iy) = image->interpolate(px, py);
					}
				}
			}
		}
	}
}

template<class T>
void SunImage<T>::Lambert_cylindrical_projection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	T* j = this->pixels;
	
	Real dy = 2. / Real(this->yAxes), dx = PI / Real(this->xAxes);
	
	if(exact)
	{
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = asin((py * dy) - 1.);
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = (px * dx) - MIPI;
				*j = image->interpolate(HGS(longitude, latitude));
				++j;
			}
		}
	}
	else
	{
		Real sun_radius = image->SunRadius();
		RealPixLoc sun_center = image->SunCenter();
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = asin((py * dy) - 1.);
			Real cos_lat = cos(latitude);
			Real iy = sun_center.y + sin(latitude) * sun_radius;
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = (px * dx) - MIPI;
				Real ix = sun_center.x + (sun_radius * cos_lat * sin(longitude));
				*j = image->interpolate(ix, iy);
				++j;
			}
		}
	}
}


template<class T>
void SunImage<T>::Lambert_cylindrical_deprojection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	
	Real dy = Real(image->Yaxes()) / 2., dx = Real(image->Xaxes()) / PI;
	
	if(exact)
	{
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			for(unsigned ix = 0; ix < this->xAxes; ++ix)
			{
				HGS hgs = this->toHGS(RealPixLoc(ix,iy));
				if(!(!hgs))
				{
					Real px = (hgs.longitude + MIPI) * dx;
					Real py = (sin(hgs.latitude) + 1.) * dy;
					this->pixel(ix,iy) = image->interpolate(px, py);
				}
			}
		}
	}
	else
	{
		Real sun_radius = SunRadius();
		RealPixLoc sun_center = SunCenter();
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			Real ry = Real(iy - sun_center.y) / sun_radius;
			if(ry <= 1. && ry >= -1.)
			{
				Real latitude = asin(ry);
				Real cos_lat = cos(latitude);
				for(unsigned ix = 0; ix < this->xAxes; ++ix)
				{
					Real rx = Real(ix - sun_center.x) / sun_radius;
					if(rx*rx + ry*ry <= 1)
					{
						Real longitude = asin(rx / cos_lat);
						Real px = (longitude + MIPI) * dx;
						Real py = (sin(latitude) + 1.) * dy;
						this->pixel(ix, iy) = image->interpolate(px, py);
					}
				}
			}
		}
	}
}


template<class T>
void SunImage<T>::sinuosidal_projection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	T* j = this->pixels;
	
	Real dy = PI / Real(this->yAxes), dx = PI / Real(this->xAxes);
	
	if(exact)
	{
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = (py * dy) - MIPI;
			Real cos_lat = cos(latitude);
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = ((px * dx) - MIPI) / cos_lat;
				if(-MIPI <= longitude && longitude <= MIPI)
					*j = image->interpolate(HGS(longitude, latitude));
				++j;
			}
		}
	}
	else
	{
		Real sun_radius = image->SunRadius();
		RealPixLoc sun_center = image->SunCenter();
		for(unsigned py = 0; py < this->yAxes; ++py)
		{
			Real latitude = (py * dy) - MIPI;
			Real cos_lat = cos(latitude);
			Real iy = sun_center.y + (sin(latitude) * sun_radius);
			for(unsigned px = 0; px < this->xAxes; ++px)
			{
				Real longitude = ((px * dx) - MIPI) / cos_lat;
				if(-MIPI <= longitude && longitude <= MIPI)
				{
					Real ix = sun_center.x + (sun_radius * cos_lat * sin(longitude));
					*j = image->interpolate(ix, iy);
				}
				++j;
			}
		}
	}
}


template<class T>
void SunImage<T>::sinuosidal_deprojection(const SunImage<T>* image, bool exact)
{
	this->zero(this->null());
	
	Real dy = Real(image->Yaxes()) / PI, dx = Real(image->Xaxes()) / PI;
	if(exact)
	{
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			for(unsigned ix = 0; ix < this->xAxes; ++ix)
			{
				HGS hgs = this->toHGS(RealPixLoc(ix,iy));
				if(!(!hgs))
				{
					Real px = ((hgs.longitude * cos(hgs.latitude)) + MIPI) * dx;
					Real py = (hgs.latitude + MIPI) * dy;
					this->pixel(ix,iy) = image->interpolate(px, py);
				}
			}
		}
	}
	else
	{
		Real sun_radius = SunRadius();
		RealPixLoc sun_center = SunCenter();
		for(unsigned iy = 0; iy < this->yAxes; ++iy)
		{
			Real ry = Real(iy - sun_center.y) / sun_radius;
			if(ry <= 1. && ry >= -1.)
			{
				Real latitude = asin(ry);
				Real cos_lat = cos(latitude);
				for(unsigned ix = 0; ix < this->xAxes; ++ix)
				{
					Real rx = Real(ix - sun_center.x) / sun_radius;
					if(rx*rx + ry*ry <= 1)
					{
						Real longitude = asin(rx / cos_lat);
						Real px = ((longitude * cos_lat) + MIPI) * dx;
						Real py = (latitude + MIPI) * dy;
						this->pixel(ix, iy) = image->interpolate(px, py);
					}
				}
			}
		}
	}
}

template class SunImage<EUVPixelType>;
template class SunImage<ColorType>;

/*! Formula coming from Rotation of Doppler features in the solar photosphere by Snodgrass, Herschel B. and Ulrich, Roger K.
	@return The average angular speed in radians/seconds
*/
inline Real SunDifferentialAngularSpeed(const Real& latitude)
{

	const Real A = 14.71;
	const Real B = -2.39;
	const Real C =  -1.78;
	Real sin_latitude_squared = sin(latitude);
	sin_latitude_squared *= sin_latitude_squared;
	return (A + (B + C * sin_latitude_squared) * sin_latitude_squared) * DEGREE2RADIAN / (24 * 3600);
}

//! Routine that computes the julian day number from a time_t
Real julian_day(const time_t& observationTime)
{
	// Transform observationTime to year/month/day
	tm* date = gmtime(&observationTime);
	int year = date->tm_year + 1900;
	int month = date->tm_mon + 1;
	
	// Compute the julian day
	int a = (14 - month)/12;
	int y = year + 4800 - a;
	int m = month + 12 * a - 3;
	
	Real jdn = int(date->tm_mday + (153*m+2)/5 + y*365 + y/4 - y/100 + y/400 - 32045) + ((date->tm_hour - 12.) / 24. + (date->tm_min / 1440.) + (date->tm_sec/86400.));
	return jdn;
}

/*! Code taken from the Solar Soft procedure sun_position */
void sun_position(const Real& jdn, Real& longitude, Real& ra, Real& dec, Real& app_longitude, Real& obliquity)
{
	double t = jdn/36525.0;
	#if DEBUG >= 3
		cout<<fixed;
		cout<<"Julian centuries: "<<t<<endl;
	#endif
	app_longitude = (279.696678 + fmod(36000.768925*t,360.0))*3600.0;
	#if DEBUG >= 3
		cout<<"Sun mean longitude: "<<app_longitude<<endl;
	#endif
	double me = 358.475844 + fmod(35999.049750 * t, 360.0);
	double ellcor = (6910.1 - 17.2*t)*sin(me * DEGREE2RADIAN) + 72.3 * sin(2.0 * me* DEGREE2RADIAN);
	app_longitude = app_longitude + ellcor;
	#if DEBUG >= 3
		cout<<"Earth ellipticity correction: "<<app_longitude<<endl;
	#endif

	double mv = 212.603219 + fmod(58517.803875*t, 360.0) ;
	double vencorr = 4.8 * cos((299.1017 + mv - me)* DEGREE2RADIAN) + 5.5 * cos((148.3133 + 2.0 * mv - 2.0 * me)* DEGREE2RADIAN) + 2.5 * cos((315.9433 + 2.0 * mv - 3.0 * me)* DEGREE2RADIAN) + 1.6 * cos((345.2533 + 3.0 * mv - 4.0 * me)* DEGREE2RADIAN) + 1.0 * cos((318.15 + 3.0 * mv - 5.0 * me)* DEGREE2RADIAN);
	app_longitude = app_longitude + vencorr;
	#if DEBUG >= 3
		cout<<"Venus perturbation correction: "<<app_longitude<<endl;
	#endif

	double mm = 319.529425 + fmod(19139.858500 * t, 360.0);
	double marscorr = 2.0 * cos((343.8883 - 2.0 * mm + 2.0 * me)* DEGREE2RADIAN) + 1.8 * cos((200.4017 - 2.0 * mm + me) * DEGREE2RADIAN);
	app_longitude = app_longitude + marscorr;
	#if DEBUG >= 3
		cout<<"Mars perturbation correction: "<<app_longitude<<endl;
	#endif

	double mj = 225.328328 + fmod(3034.6920239 * t, 360.0);
	double jupcorr = 7.2 * cos((179.5317 - mj + me)* DEGREE2RADIAN) + 2.6 * cos((263.2167 - mj) * DEGREE2RADIAN) + 2.7 * cos((87.1450 - 2.0 * mj + 2.0 * me) * DEGREE2RADIAN) + 1.6 * cos((109.4933 - 2.0 * mj + me) * DEGREE2RADIAN);
	app_longitude = app_longitude + jupcorr;
	#if DEBUG >= 3
		cout<<"Jupiter perturbation correction: "<<app_longitude<<endl;
	#endif
	double d = 350.7376814 + fmod(445267.11422 * t, 360.0);
	double mooncorr = 6.5 * sin(d* DEGREE2RADIAN);
	app_longitude = app_longitude + mooncorr;
	#if DEBUG >= 3
		cout<<"Moon perturbation correction: "<<app_longitude<<endl;
	#endif
	double longterm = + 6.4 * sin((231.19 + 20.20 * t)* DEGREE2RADIAN);
	app_longitude = app_longitude + longterm;
	app_longitude = fmod(app_longitude + 2592000.0, 1296000.0);
	#if DEBUG >= 3
		cout<<"Long period terms correction: "<<app_longitude<<endl;
	#endif
	longitude = app_longitude/3600.0;
	
	app_longitude = app_longitude - 20.5;
	#if DEBUG >= 3
		cout<<"Aberration correction: "<<app_longitude<<endl;
	#endif
	double omega = 259.183275 - fmod(1934.142008 * t , 360.0);
	app_longitude = app_longitude - 17.2 * sin(omega* DEGREE2RADIAN);
	#if DEBUG >= 3
		cout<<"Nutation correction: "<<app_longitude<<endl;
	#endif
	obliquity = 23.452294 - 0.0130125*t + (9.2*cos(omega* DEGREE2RADIAN))/3600.0;
	app_longitude = app_longitude/3600.0;
	ra = atan2(sin(app_longitude* DEGREE2RADIAN) * cos(obliquity* DEGREE2RADIAN) , cos(app_longitude* DEGREE2RADIAN)) * RADIAN2DEGREE;
	if (ra < 0.0)
		ra += 360.0;
	dec = asin(sin(app_longitude* DEGREE2RADIAN) * sin(obliquity* DEGREE2RADIAN)) * RADIAN2DEGREE;
}


/*! Code taken from the Solar Soft procedure pb0r
	@return The dist in Mmeters
*/
Real distance_sun_earth(const time_t& time_obs)
{
	Real jdn = julian_day(time_obs);

	// Number of Julian days since 2415020.0
	jdn -= 2415020.0;
	
	double t = jdn/36525.0;
	double mv = 212.6 + fmod(58517.80 * t, 360.0);
	double me = 358.476 + fmod(35999.0498 * t, 360.0);
	double mm = 319.5 + fmod(19139.86 * t, 360.0);
	double mj = 225.3 + fmod(3034.69 * t, 360.0);
	double d = 350.7 + fmod(445267.11 * t, 360.0);
	double r = 1.000141 - (0.016748 - 0.0000418*t) * cos(me*DEGREE2RADIAN) - 0.000140 * cos(2.0*me*DEGREE2RADIAN) + 0.000016 * cos((58.3 + 2.0*mv - 2.0*me)*DEGREE2RADIAN) + 0.000005 * cos((209.1 + mv - me)*DEGREE2RADIAN) + 0.000005 * cos((253.8 - 2.0*mm + 2.0*me)*DEGREE2RADIAN) + 0.000016 * cos((89.5 - mj + me)*DEGREE2RADIAN) + 0.000009 * cos((357.1 - 2.0*mj + 2.0*me)*DEGREE2RADIAN) + 0.000031 * cos(d*DEGREE2RADIAN);
	// Distance to the sun in arcmin
	double dist = asin((695508000.0 / 149597870691.0)/r)*10800./PI;
	#if DEBUG >= 3
		cout<<"distance_sun_earth (arcmin): "<<dist<<endl;
	#endif
	// We convert in Mmeters
	return SUN_RADIUS * (60. * 180. / PI) / dist;
}


/*! Code taken from the Solar Soft procedure pb0r

	@return The latitude in degrees
*/
Real earth_latitude(const time_t& time_obs)
{
	Real jdn = julian_day(time_obs);

	// Number of Julian days since 2415020.0
	jdn -= 2415020.0;
	
	// Compute the sun position
	Real longitude, ra, dec, app_longitude, obliquity;
	sun_position(jdn, longitude, ra, dec, app_longitude, obliquity);
	
	double lambda = longitude - (20.5/3600.0);
	double node = 73.666666 + (50.25/3600.0)*((jdn/365.25) + 50.0);
	double arg = lambda - node;
	//p = (atan(-tan(obliquity*DEGREE2RADIAN) * cos(app_longitude*DEGREE2RADIAN)) + atan(-0.12722 * cos(arg*DEGREE2RADIAN))) * RADIAN2DEGREE;
	Real b = asin(0.12620 * sin(arg*DEGREE2RADIAN)) * RADIAN2DEGREE;
	return b;
}



