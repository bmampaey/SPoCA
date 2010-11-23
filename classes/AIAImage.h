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
	public :
		
		//Constructors and destructors
		AIAImage(const std::string& filename);
		AIAImage(const SunImage& i);
		AIAImage(const SunImage* i);
		~AIAImage();
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);

		

};

bool isAIA(const FitsHeader& header);
#endif
