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
		AIAImage(const SunImage& i);
		AIAImage(const SunImage* i);
		~AIAImage();
		
		//Routines to read and write the keywords from/to the header
		void readKeywords();
		void writeKeywords();
		
		//Routines for the preprocessing on SunImages
		Real percentCorrection(const Real r) const;
		

};

bool isAIA(const FitsHeader& header);
#endif
