#pragma once
#ifndef ColorMap_H
#define ColorMap_H

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

class ColorMap : public SunImage
{
	
	public :
		
		//Constructors and destructors
		ColorMap(const std::string& filename);
		ColorMap(const long xAxes = 0, const long yAxes = 0);
		ColorMap(const SunImage& i);
		ColorMap(const SunImage* i);
		~ColorMap();
		
		
		//Routines to read and write the keywords from/to the header
		void readHeader(fitsfile* fptr);
		void writeHeader(fitsfile* fptr);

};

bool isColorMap(const FitsHeader& header);
#endif
