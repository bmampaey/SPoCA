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
SunImage<T>::SunImage(const unsigned xAxes, const unsigned yAxes)
:Image<T>(xAxes,yAxes),radius(0),observationTime(0),cdelt1(0),cdelt2(0), b0(0)
{
	suncenter.x = xAxes/2;
	suncenter.y = yAxes/2;
}

template<class T>
SunImage<T>::SunImage(const unsigned xAxes, const unsigned yAxes, const Coordinate suncenter, const double radius)
:Image<T>(xAxes,yAxes),radius(radius),observationTime(0),suncenter(suncenter),cdelt1(1),cdelt2(1),b0(0)
{
}

template<class T>
SunImage<T>::SunImage(const SunImage& i)
:Image<T>(i),radius(i.radius),observationTime(i.observationTime),suncenter(i.suncenter),cdelt1(i.cdelt1),cdelt2(i.cdelt2),date_obs(i.date_obs),b0(i.b0),header(i.header)
{
}


template<class T>
SunImage<T>::SunImage(const SunImage* i)
:Image<T>(i),radius(i->radius),observationTime(i->observationTime),suncenter(i->suncenter),cdelt1(i->cdelt1),cdelt2(i->cdelt2), date_obs(i->date_obs), b0(i->b0), header(i->header)
{
}

template<class T>
SunImage<T>::SunImage(const Header& header)
:Image<T>(),header(header)
{
}

template<class T>
Header& SunImage<T>::getHeader()
{
	preWrite();
	return header;
}

template<class T>
Coordinate SunImage<T>::SunCenter() const
{return suncenter;}
template<class T>
double SunImage<T>::SunRadius() const
{return radius;}
template<class T>
double SunImage<T>::B0() const
{return b0;}
template<class T>
double SunImage<T>::PixelLength() const
{return cdelt1;}
template<class T>
double SunImage<T>::PixelWidth() const
{return cdelt2;}

template<class T>
time_t SunImage<T>::ObservationTime()const
{
	if(observationTime == 0 && !date_obs.empty())
	{
		return iso2ctime(date_obs);
	}
	return observationTime;
}

template<class T>
string SunImage<T>::ObservationDate() const
{return date_obs;}
template<class T>
double SunImage<T>::PixelArea() const
{return cdelt1 * cdelt2;}
template<class T>
unsigned SunImage<T>::numberValidPixelsEstimate() const
{return unsigned(PI*radius*radius);}

template<class T>
void SunImage<T>::nullifyAboveRadius(const Real radiusRatio)
{
	Real radius2 = radiusRatio*radiusRatio*radius*radius;
	for (unsigned y=0; y < this->yAxes; ++y)
	{
		for (unsigned x=0; x < this->xAxes; ++x)
		{
			if ((x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y)> radius2)
				Image<T>::pixel(x,y) = this->nullvalue_;
		}
	}
}

template<class T>
void SunImage<T>::recenter(const Coordinate& newCenter)
{
	int delta = (suncenter.x - newCenter.x) + (suncenter.y - newCenter.y) * this->xAxes;
	if(delta < 0)
	{
		memmove(this->pixels - delta, this->pixels, (this->numberPixels + delta + 1) * sizeof(T));
		fill(this->pixels, this->pixels-delta, this->nullvalue_);
	}
	else if (delta > 0)
	{
		memmove(this->pixels, this->pixels + delta, (this->numberPixels - delta + 1) * sizeof(T));
		fill(this->pixels + this->numberPixels - delta, this->pixels + this->numberPixels, this->nullvalue_);
	}
	suncenter = newCenter;
}

template<class T> template<class T2>
void SunImage<T>::copyKeywords(const SunImage<T2>* i)
{

	this->radius = i->SunRadius();
	this->suncenter = i->SunCenter();
	this->observationTime = i->ObservationTime();
	this->cdelt1 = i->PixelLength();
	this->cdelt2 = i->PixelWidth();
	this->date_obs =  i->ObservationDate();
	this->b0 = i->B0();
}

// Calculates the differential solar rotation speed for a given pixel
// Formula coming from Rotation of Doppler features in the solar photosphere by	Snodgrass, Herschel B. and Ulrich, Roger K.
template<class T>
inline Real SunImage<T>::angularSpeed(Real latitude) const
{
	const Real A = 14.713;
	const Real B = -2.396;
	const Real C =  -1.787;
	latitude *= latitude;
	Real result = A + B * latitude + C * latitude * latitude;
	return result * (PI / (24. * 3600. * 180.));
}

// Rotate an image by delta_t seconds
template<class T>
void SunImage<T>::rotate(const int delta_t)
{
	T* new_pixels = new T[this->numberPixels];
	fill(new_pixels, new_pixels + this->numberPixels, this->nullvalue_);
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);

	//Compute for each point of the new image what is the equivalent in the current image
	
	unsigned j = 0;
	for(Real y = 0; y < this->yAxes; ++y)
	{
		Real ry = (y - suncenter.y) / radius;
		for(Real x = 0; x < this->xAxes; ++x)
		{
			Real rx = (x - suncenter.x) / radius;
			Real z = 1. - rx * rx - ry * ry;
			// If we are within the disk
			if(z >= 0)
			{ 
				z = sqrt(z);
				Real cur_latitude = asin(cos_b0 * ry + sin_b0 * z); // The new latitude equals the current latitude
				Real new_longitude = atan(rx / (cos_b0 * z - sin_b0 * ry));
				Real cur_longitude = new_longitude - delta_t * angularSpeed(cur_latitude);
				// We need to be sure that the current longitude is visible on the disk
				if(cur_longitude > -MIPI &&  cur_longitude < MIPI)
				{
					unsigned cur_x = suncenter.x + radius * (cos(cur_latitude) * sin(cur_longitude));
					unsigned cur_y = suncenter.y + radius * (cos_b0 * sin(cur_latitude) - sin_b0 * cos(cur_latitude) * cos(cur_longitude));
					new_pixels[j] = Image<T>::pixel(cur_x, cur_y);
				}
			}
 			++j;
		}
	}
	delete this->pixels;
	this->pixels = new_pixels;
}

// Return a new image = to the image rotated to be comparable to img
template<class T>
SunImage<T>* SunImage<T>::rotated_like(const SunImage* img) const
{
	SunImage * rotated = new SunImage(img->xAxes, img->yAxes);
	rotated->copyKeywords(img);
	rotated->nullvalue_ = img->nullvalue_;
	rotated->zero(this->nullvalue_);
	
	int delta_t = difftime(img->ObservationTime(),ObservationTime());
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	Real cos_newb0 = cos(rotated->b0);
	Real sin_newb0 = sin(rotated->b0);
	
	//Compute for each point of the new image what is the equivalent in the current image
	for(Real new_y = 0; new_y < rotated->yAxes; ++new_y)
	{
		Real ry = (new_y - rotated->suncenter.y) / rotated->radius;
		for(Real new_x = 0; new_x < rotated->xAxes; ++new_x)
		{
			Real rx = (new_x - rotated->suncenter.x) / rotated->radius;
			Real z = 1. - rx * rx - ry * ry;
			// If we are within the disk
			if(z >= 0)
			{ 
				z = sqrt(z);
				Real cur_latitude = asin(cos_newb0 * ry + sin_newb0 * z); // The new latitude equals the current latitude
				Real new_longitude = atan(rx / (cos_newb0 * z - sin_newb0 * ry));
				Real cur_longitude = new_longitude - delta_t * angularSpeed(cur_latitude);
				// We need to be sure that the current longitude is visible on the disk
				if(cur_longitude > -MIPI &&  cur_longitude < MIPI)
				{
					unsigned cur_x = suncenter.x + radius * (cos(cur_latitude) * sin(cur_longitude));
					unsigned cur_y = suncenter.y + radius * (cos_b0 * sin(cur_latitude) - sin_b0 * cos(cur_latitude) * cos(cur_longitude));
					rotated->pixel(new_x,new_y) = Image<T>::pixel(cur_x, cur_y);
				}
 			}
 			
		}
	}

	return rotated;

}

// Rotate the image to be comparable to img
template<class T>
void SunImage<T>::rotate_like(const SunImage* img)
{
	SunImage * rotated = this->rotated_like(img);
	this->xAxes = rotated->xAxes;
	this->yAxes = rotated->yAxes;
	this->numberPixels = rotated->numberPixels;
	this->nullvalue_ = rotated->nullvalue_;
	delete this->pixels;
	this->pixels = rotated->pixels;

}

// Shift a point in the image by delta_t seconds
template<class T>
Coordinate SunImage<T>::shift(const Coordinate c, const int delta_t) const
{

	Real latitude, longitude;
	longlat(c, longitude, latitude);
	if(isnan(longitude) || isnan(latitude))
		return Coordinate::Max;
	
	longitude += delta_t * angularSpeed(latitude);
	
	Coordinate newc;
	newc.x = suncenter.x + radius * (cos(latitude) * sin(longitude));
	newc.y = suncenter.y + radius * (sin(latitude) * cos(b0) - cos(latitude) * cos(longitude) * sin(b0));
	
	return newc;

}

// Shift a point in the image to the equivalant point in img
template<class T>
Coordinate SunImage<T>::shift_like(const Coordinate c, const SunImage* img) const
{

	int delta_t = difftime(img->ObservationTime(),ObservationTime());
	Real latitude, longitude;
	longlat(c, longitude, latitude);
	if(isnan(longitude) || isnan(latitude))
		return Coordinate::Max;
	longitude += delta_t * angularSpeed(latitude);
	
	Coordinate newc;
	newc.x = img->suncenter.x + img->radius * (cos(latitude) * sin(longitude));
	newc.y = img->suncenter.y + img->radius * (sin(latitude) * cos(img->b0) - cos(latitude) * cos(longitude) * sin(img->b0));
	
	return newc;

}


template<class T>
void SunImage<T>::longlat(const Coordinate c, Real& longitude, Real& latitude) const
{
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	Real rx = c.x ; 
	Real ry = c.y;
	rx = (rx - suncenter.x) / radius; 
	ry = (ry - suncenter.y) / radius;
	Real z = 1. - rx * rx - ry * ry;
	if(z >= 0)
	{ 
		z = sqrt(z);
		latitude = asin(cos_b0 * ry + sin_b0 * z);
		longitude = atan(rx / (cos_b0 * z - sin_b0 * ry));
	}
	else
	{
		latitude = NAN;
		longitude = NAN;
	}
}

template<class T>
void SunImage<T>::longlat_map(vector<Real>& longitude_map, vector<Real>& latitude_map) const
{

	longitude_map.resize(this->numberPixels, 0);
	latitude_map.resize(this->numberPixels, 0);
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	
	unsigned j = 0;
	for(Real y = 0; y < this->yAxes; ++y)
	{
		Real ry = (y - suncenter.y) / radius;
		for(Real x = 0; x < this->xAxes; ++x)
		{
			Real rx = (x - suncenter.x) / radius;
			Real z = 1. - rx * rx - ry * ry;
			if(z >= 0)
			{ 
				z = sqrt(z);
				latitude_map[j] = asin(cos_b0 * ry + sin_b0 * z);
				longitude_map[j] = atan(rx / (cos_b0 * z - sin_b0 * ry));
 			}
 			else
			{
				latitude_map[j] = NAN;
				longitude_map[j] = NAN;
			}
			++j;
		}
	}


}

template<class T>
FitsFile& SunImage<T>::writeFits(FitsFile& file, int mode)
{
	preWrite();
	Image<T>::writeFits(file, mode);
	file.writeHeader(header);
	return file;
}

template<class T>
FitsFile& SunImage<T>::readFits(FitsFile& file)
{
	file.readHeader(header);
	Image<T>::readFits(file);
	postRead();
	return file;
}

template<class T>
bool SunImage<T>::writeFits(const std::string& filename, int mode)
{
	FitsFile file(filename, FitsFile::overwrite);
	this->writeFits(file, mode);
	return file.isGood();
}

template<class T>
bool SunImage<T>::readFits(const std::string& filename)
{
	FitsFile file(filename);
	this->readFits(file);
	return file.isGood();
}


template class SunImage<PixelType>;
template class SunImage<ColorType>;
template void SunImage<PixelType>::copyKeywords(const SunImage<PixelType>* i);
template void SunImage<ColorType>::copyKeywords(const SunImage<ColorType>* i);
template void SunImage<PixelType>::copyKeywords(const SunImage<ColorType>* i);
template void SunImage<ColorType>::copyKeywords(const SunImage<PixelType>* i);
 
