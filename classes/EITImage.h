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


};

bool isEIT(const FitsHeader& header);
#endif
