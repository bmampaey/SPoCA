#pragma once
#ifndef EUVImage_H
#define EUVImage_H

#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>

#include "Coordinate.h"
#include "SunImage.h"



class EUVImage : public SunImage<PixelType>
{

	protected :
		double wavelength;
		double median;
		double mode;
		double datap01,datap95;
		double exposureTime;
		Real sineCorrectionParameters[4];
	
		virtual Real MINRADIUS()
		{ return sineCorrectionParameters[0]; }

	public :
		
		//Constructors and destructors
		EUVImage(const long xAxes = 0, const long yAxes = 0);
		EUVImage(const long xAxes, const long yAxes, const Coordinate suncenter, const double radius);
		EUVImage(const EUVImage& i);
		EUVImage(const EUVImage* i);
		EUVImage(const Header& header);
		~EUVImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
		
		//Accessors
		double Wavelength() const;
		double Median() const;
		double ExposureTime() const;
		void copyKeywords(const EUVImage* i);

		//Routines for the preprocessing on EUVImages
		void preprocessing(std::string preprocessingList, const Real radiusRatio = 1.0);
		void annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius);
		void ALCDivMedian(Real maxLimbRadius, Real minLimbRadius);
		void ALCDivMode(Real maxLimbRadius, Real minLimbRadius);
		Real percentCorrection(const Real r) const;

};

#endif
