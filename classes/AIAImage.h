#pragma once
#ifndef AIAImage_H
#define AIAImage_H

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

class AIAImage : public SunImage
{

	Real MINRADIUS()
	{ return AIA_SINE_CORR_R1 / 100.; }

	public :
		
		//Constructors and destructors
		AIAImage(const std::string& filename);
		AIAImage(const long xAxes = 0, const long yAxes = 0, const double radius = 0., const double wavelength = 0.);
		AIAImage(const SunImage& i);
		AIAImage(const SunImage* i);
		~AIAImage();
		
		//Routines to read and write a fits file
		virtual int readFitsImageP(fitsfile* fptr);
		
		//Routines for the preprocessing on SunImages
		Real percentCorrection(const Real r) const;
		

};
#endif
