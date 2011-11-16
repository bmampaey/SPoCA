#pragma once
#ifndef EITImage_H
#define EITImage_H

#include "EUVImage.h"
#include "Header.h"

class EITImage : public EUVImage
{
	public :
		//! Constructor
		EITImage();
		
		//! Constructor for an EITImage from an header
		EITImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an EITImage from a WCS
		EITImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		EITImage(const EUVImage& i);
		
		//! Copy Constructor
		EITImage(const EUVImage* i);
		
		//! Destructors
		~EITImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Routine that improves the contrast for visualization
		/*! Based on the routine eit_data_scale.pro from solar soft */
		void enhance_contrast();
		
		//! Routine that returns the color table
		std::vector<char> color_table() const;
		
		//! Accessor to retrieve the Instrument
		std::string Instrument() const;
};

bool isEIT(const Header& header);
#endif
