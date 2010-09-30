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
		virtual void readKeywords();
		virtual void writeKeywords();
		
		//Accessors
		double Wavelength() const;
		double Median() const;
		Coordinate SunCenter() const;
		double SunRadius() const;
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
		
		
		//Future routines to derotate an image (in progress) 
		/*
		Real angularSpeed(Real latitude);
		unsigned newPos(Real x, Real y, const Real t);
		SunImage* rotate(const unsigned t);
		*/
		
};

const Real PI = 3.14159265358979323846;
const Real MIPI = 1.57079632679489661923;
const Real BIPI = 6.28318530717958647692;
#endif
