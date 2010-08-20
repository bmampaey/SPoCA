// To use this class you need to have installed open-motif libraries libmotif3 and lesstif2

#pragma once
#ifndef CoordinateConvertor_H
#define CoordinateConvertor_H


#warning "To use this class you need to have installed open-motif libraries libmotif3 and lesstif2"
#warning "You should also make sure to set your solar soft path (SSW_PATH) in the constant.h file"

#include <limits>
#include <iostream>
#include <string>
#include <cstdlib>

#include "idl_export.h"
#include "Coordinate.h"
#include "SunImage.h"

	/*Possible types are
	Helioprojective-Cartesian       (HPC)
	Helioprojective-Radial          (HPR)
	Heliocentric-Cartesian          (HCC)
	Heliocentric-Radial             (HCR)
	Stonyhurst-Heliographic         (HG)
	Carrington-Heliographic         (HG, /CARRINGTON)
	*/

class CoordinateConvertor
{
	private :
		static unsigned instances;
		static unsigned lastId;
		std::string  wcs;
		std::string coordinateType;

	public :
		//Constructors
		CoordinateConvertor(SunImage* image);
		CoordinateConvertor(SunImage* image, std::string coordinateType);
		~CoordinateConvertor();
		CoordinateConvertor(const CoordinateConvertor& cc);
		void convert(std::string coord_type, Coordinate c, float& x, float& y, bool arcsec = false) const;
		void convert(Coordinate c, float& x, float& y, bool arcsec = false) const;
};

#endif
		
		
