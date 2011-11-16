#pragma once
#ifndef SWAPImage_H
#define SWAPImage_H

#include "EUVImage.h"
#include "Header.h"

class SWAPImage : public EUVImage
{
	public :
		//! Constructor
		SWAPImage();

		//! Constructor for an SWAPImage from an header
		SWAPImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an SWAPImage from a WCS
		SWAPImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		SWAPImage(const EUVImage& i);
		
		//! Copy Constructor
		SWAPImage(const EUVImage* i);
		
		//! Destructors
		~SWAPImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
};

bool isSWAP(const Header& header);
#endif
