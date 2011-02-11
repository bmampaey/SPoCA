#pragma once
#ifndef HMIImage_H
#define HMIImage_H

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

class HMIImage : public SunImage
{
	public :
		
		//Constructors and destructors
		HMIImage(const std::string& filename);
		HMIImage(const SunImage& i);
		HMIImage(const SunImage* i);
		~HMIImage();
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);

		

};

bool isHMI(const FitsHeader& header);
#endif

