#pragma once
#ifndef SWAPImage_H
#define SWAPImage_H

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

class SWAPImage : public SunImage
{

	Real MINRADIUS()
	{ return SWAP_SINE_CORR_R1 / 100.; }
	
	public :
		
		//Constructors and destructors
		SWAPImage(const std::string& filename);
		SWAPImage(const long xAxes = 0, const long yAxes = 0, const double radius = 0., const double wavelength = 0.);
		SWAPImage(const SunImage& i);
		SWAPImage(const SunImage* i);
		~SWAPImage();
		
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);
		
		//Routines for the preprocessing on SunImages
		Real percentCorrection(const Real r) const;

};

bool isSWAP(const FitsHeader& header);
#endif
