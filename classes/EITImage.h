#pragma once
#ifndef EITImage_H
#define EITImage_H

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

class EITImage : public SunImage
{

	Real MINRADIUS()
	{ return EIT_SINE_CORR_R1 / 100.; }
	
	public :
		
		//Constructors and destructors
		EITImage(const std::string& filename);
		EITImage(const long xAxes = 0, const long yAxes = 0, const double radius = 0., const double wavelength = 0.);
		EITImage(const SunImage& i);
		EITImage(const SunImage* i);
		~EITImage();
		
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);
		
		
		//Routines for the preprocessing on SunImages
		Real percentCorrection(const Real r) const;

};

bool isEIT(const FitsHeader& header);
#endif
