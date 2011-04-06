#pragma once
#ifndef SunImage_H
#define SunImage_H

#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>

#include "Image.h"
#include "Coordinate.h"
#include "Header.h"
#include "FitsFile.h"


template<class T>
class SunImage : public Image<T>
{
	protected :
		//! Radius of the sun
		double radius;
		
		//! Time of observation
		time_t observationTime;
		
		//! Location of the center of the sun in the image
		Coordinate suncenter;
		
		//! Length of pixel in arcsec
		double cdelt1;
		
		//! Width of pixel in arcsec
		double cdelt2;
		
		//! Time of observation as a string
		std::string date_obs;
		
		//! B0 to be documented
		double b0;
	
	public:
	
		//! A header containing all keywords when the image is read from a fits file
		Header header;
	
	public :
		
		//! Constructor for an SunImage of size xAxes x yAxes
		SunImage(const unsigned xAxes = 0, const unsigned yAxes = 0);
		
		//! Constructor for an SunImage of size xAxes x yAxes, with sun center and radius
		SunImage(const unsigned xAxes, const unsigned yAxes, const Coordinate suncenter, const double radius);
		
		//! Constructor for an SunImage of size xAxes x yAxes, with an header
		SunImage(const Header& header);
		
		//! Copy Constructor
		SunImage(const SunImage& i);
		
		//! Copy Constructor
		SunImage(const SunImage* i);
		
		//! Destructors
		~SunImage();
		
		//! Accessor to retrieve the SunCenter
		Coordinate SunCenter() const;
		//! Accessor to retrieve the SunRadius
		double SunRadius() const;
		//! Accessor to retrieve the B0
		double B0() const;
		//! Accessor to retrieve the ObservationTime
		time_t ObservationTime() const;
		//! Accessor to retrieve the ObservationDate
		std::string ObservationDate() const;
		//! Accessor to retrieve the PixelLength in arcsec (i.e. cdelt1)
		double PixelLength() const;
		//! Accessor to retrieve the PixelWidth in arcsec (i.e. cdelt2)
		double PixelWidth() const;
		//! Accessor to retrieve the PixelArea in arcsec² (i.e. PixelLength * PixelWidth) 
		double PixelArea() const;
		//! Accessor to retrieve an estimation of the number of valid pixels
		unsigned numberValidPixelsEstimate() const;
		//! Accessor to retrieve the header
		Header& getHeader();

		//! Routines to read the sun parameters from the header
		virtual void postRead(){};
		
		//! Routines to write the sun parameters to the header
		virtual void preWrite(){};
		
		//! Routine to copy the parameters from another SunImage
		template<class T2>
		void copySunParameters(const SunImage<T2>* i);
		
		//! Routine to check if the SunImage have similar parameter
		template<class T2>
		bool checkSimilar(const SunImage<T2>* image) const;
		
		//! Routine to set the pixels above a certain radius ratio to nullvalue
		void nullifyAboveRadius(const Real radiusRatio = 1.0);

		//! Routine to align the SunImage on the newCenter
		void recenter(const Coordinate& newCenter);
		
		//! Routine that compute the diffrential rotation speed of the sun for a particular latitude 
		Real angularSpeed(Real latitude) const;
		
		//! Routine that rotate the SunImage by delta_t seconds
		void rotate(const int delta_t);
		
		//! Routine that returns a SunImage similar to img
		/*! Allocate memory for the returned SunImage*/
		SunImage* rotated_like(const SunImage* img) const;
		
		//! Routine that rotate the SunImage to be similar to img
		void rotate_like(const SunImage* img);
		
		//! Routine that rotate a coordinate by delta_t seconds
		Coordinate shift(const Coordinate c, const int delta_t) const;
		
		//! Routine that rotate a coordinate to the same latitude and longitude of img 
		Coordinate shift_like(const Coordinate c, const SunImage* img) const;
		
		//! Routine that returns the longitude and latitude of a point 
		void longlat(const Coordinate c, Real& longitude, Real& latitude) const;
		
		//! Routine that returns the map longitude and latitude of the SunImage
		void longlat_map(std::vector<Real>& longitude_map, std::vector<Real>& latitude_map) const;
		
		//Routines to read/write fits files
		FitsFile& writeFits(FitsFile& file, int mode = 0);
		FitsFile& readFits(FitsFile& file);
		bool writeFits(const std::string& filename, int mode = 0);
		bool readFits(const std::string& filename);

};

#endif
