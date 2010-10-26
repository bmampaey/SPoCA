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

#include "fitsio.h"
#include "longnam.h"
#include "Image.h"
#include "Coordinate.h"
#include "FitsHeader.h"


class SunImage : public Image<PixelType>
{

	protected :
	
		double radius;
		double wavelength;
		time_t observationTime;
		Coordinate suncenter;
		double cdelt1, cdelt2;
		PixelType median, mode, datap01, datap95;
		std::string date_obs;
		double exposureTime;
		double b0;
	
		virtual Real MINRADIUS()
		{ return SINE_CORR_R1 / 100.; }
		
	public :
		FitsHeader header;

	public :
		
		//Constructors and destructors
		SunImage(const long xAxes = 0, const long yAxes = 0);
		SunImage(const long xAxes, const long yAxes, const Coordinate suncenter, const double radius, const double cdelt1, const double cdelt2, const double wavelength = 0.);
		SunImage(const std::string& filename);
		SunImage(const SunImage& i);
		SunImage(const SunImage* i);
		~SunImage();
		
		//Routines to read and write a fits file
          int writeFitsImageP(fitsfile* fptr);
          int readFitsImageP(fitsfile* fptr);
          
          //Routines to read and write the keywords from/to the header
		virtual void readHeader(fitsfile* fptr);
		virtual void writeHeader(fitsfile* fptr);
		
		//Accessors
		double Wavelength() const;
		double Median() const;
		Coordinate SunCenter() const;
		double SunRadius() const;
		double B0() const;
		time_t ObservationTime() const;
		std::string ObservationDate() const;
		double PixelArea() const;
		unsigned numberValidPixelsEstimate() const;

		//Routines for the preprocessing on SunImages
		void preprocessing(std::string preprocessingList, const Real radiusRatio = 1.0);
		void nullifyAboveRadius(const Real radiusRatio = 1.0);
		void annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius);
		void ALCDivMedian(Real maxLimbRadius, Real minLimbRadius);
		void ALCDivMode(Real maxLimbRadius, Real minLimbRadius);
		virtual Real percentCorrection(const Real r) const;

		//Various routines to work on SunImages
		void recenter(const Coordinate& newCenter);
		void copyKeywords(const SunImage* i);
		
		
		//Routines to derotate an image 
		Real angularSpeed(Real latitude) const;
		void rotate(const int delta_t);
		SunImage* rotated_like(const SunImage* img) const;
		void rotate_like(const SunImage* img);
		Coordinate shift(const Coordinate c, const int delta_t) const;
		Coordinate shift_like(const Coordinate c, const SunImage* img) const;
		void longlat(const Coordinate c, Real& longitude, Real& latitude) const;
		void longlat_map(std::vector<Real>& longitude_map, std::vector<Real>& latitude_map) const;
		
		
};

#endif
