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
		// Common parameters of a sun image
		double radius;
		time_t observationTime;
		Coordinate suncenter;
		double cdelt1, cdelt2;
		std::string date_obs;
		double b0;
	
	public:
		Header header;
	
	public :
		
		//Constructors and destructors
		SunImage(const unsigned xAxes = 0, const unsigned yAxes = 0);
		SunImage(const unsigned xAxes, const unsigned yAxes, const Coordinate suncenter, const double radius);
		SunImage(const Header& header);
		SunImage(const SunImage& i);
		SunImage(const SunImage* i);
		~SunImage();
		
		//Accessors
		Coordinate SunCenter() const;
		double SunRadius() const;
		double B0() const;
		time_t ObservationTime() const;
		std::string ObservationDate() const;
		double PixelLength() const;
		double PixelWidth() const;
		double PixelArea() const;
		unsigned numberValidPixelsEstimate() const;
		template<class T2>
		void copyKeywords(const SunImage<T2>* i);

		//Routines to read and write the keywords from/to the header
		virtual void postRead(){};
		virtual void preWrite(){};
		Header& getHeader();

		//Routine to set the pixels above a certain radius ration to nullvalue
		void nullifyAboveRadius(const Real radiusRatio = 1.0);

		//Various routines to work on SunImages
		void recenter(const Coordinate& newCenter);
		
		//Routines to derotate an image 
		Real angularSpeed(Real latitude) const;
		void rotate(const int delta_t);
		SunImage* rotated_like(const SunImage* img) const;
		void rotate_like(const SunImage* img);
		Coordinate shift(const Coordinate c, const int delta_t) const;
		Coordinate shift_like(const Coordinate c, const SunImage* img) const;
		void longlat(const Coordinate c, Real& longitude, Real& latitude) const;
		void longlat_map(std::vector<Real>& longitude_map, std::vector<Real>& latitude_map) const;
		
		//Routines to read/write fits files
		FitsFile& writeFits(FitsFile& file, int mode = 0);
		FitsFile& readFits(FitsFile& file);
		bool writeFits(const std::string& filename, int mode = 0);
		bool readFits(const std::string& filename);

};

#endif
