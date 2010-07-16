#pragma once
#ifndef CoordinateConvertor_H
#define CoordinateConvertor_H

#include <limits>
#include <iostream>
#include <string>

#include "idl_export.h"
#include "Coordinate.h"
#include "SunImage.h"

class CoordinateConvertor
{
	private :
		static unsigned instances;
		static unsigned lastId;
		std::string  wcs;

	public :
		//Constructors
		CoordinateConvertor(SunImage* image);
		~CoordinateConvertor();
		void convert(std::string coord_type, Coordinate c, float& x, float& y, bool arcsec = false);		
};

#endif
		
		
