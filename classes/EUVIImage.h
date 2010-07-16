#pragma once
#ifndef EUVIImage_H
#define EUVIImage_H

#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>

#include "fitsio.h"
#include "longnam.h"
#include "SunImage.h"

class EUVIImage : public SunImage
{
	
	Real MINRADIUS()
	{ return EUVI_SINE_CORR_R1 / 100.; }
	
	public :
		
		//Constructors and destructors
		EUVIImage(const std::string& filename);
		EUVIImage(const long xAxes = 0, const long yAxes = 0, const double radius = 0., const double wavelength = 0.);
		EUVIImage(const SunImage& i);
		EUVIImage(const SunImage* i);
		~EUVIImage();
		
		//Routines to read and write a fits file
		virtual int readFitsImageP(fitsfile* fptr);
		
		//Routines for the preprocessing on SunImages
		Real percentCorrection(const Real r) const;


};
#endif
