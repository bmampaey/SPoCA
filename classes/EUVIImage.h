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

	public :
		
		//Constructors and destructors
		EUVIImage(const std::string& filename);
		EUVIImage(const long xAxes = 0, const long yAxes = 0, const double radius = 0., const double wavelength = 0.);
		EUVIImage(const SunImage& i);
		EUVIImage(const SunImage* i);
		~EUVIImage();
		
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);
		

};

bool isEUVI(const FitsHeader& header);
#endif
