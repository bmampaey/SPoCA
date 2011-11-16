#pragma once
#ifndef AIAImage_H
#define AIAImage_H

#include "EUVImage.h"
#include "Header.h"
#include <vector>

class AIAImage : public EUVImage
{
	public :
		//! Constructor
		AIAImage();
		
		//! Constructor for an AIAImage from an header
		AIAImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an AIAImage from a WCS
		AIAImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		AIAImage(const EUVImage& i);
		
		//! Copy Constructor
		AIAImage(const EUVImage* i);
		
		//! Destructors
		~AIAImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Routine that improves the contrast for visualization
		/*! Based on the routine aia_intscale.pro from solar soft */
		void enhance_contrast();
		
		//! Routine that returns the color table
		std::vector<char> color_table() const;
		
		//! Accessor to retrieve the Instrument
		std::string Instrument() const;
};

bool isAIA(const Header& header);
#endif
