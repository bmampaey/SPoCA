#pragma once
#ifndef SUVIImage_H
#define SUVIImage_H

#include "EUVImage.h"
#include "Header.h"
#include <vector>

class SUVIImage : public EUVImage
{
	public :
		//! Constructor
		SUVIImage();
		
		//! Constructor for an SUVIImage from an header
		SUVIImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an SUVIImage from a WCS
		SUVIImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		SUVIImage(const EUVImage& i);
		
		//! Copy Constructor
		SUVIImage(const EUVImage* i);
		
		//! Destructors
		~SUVIImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Routine that improves the contrast for visualization
		void enhance_contrast();
		
		//! Routine that returns the color table
		std::vector<unsigned char> color_table() const;
		
		//! Accessor to retrieve the Instrument
		std::string Instrument() const;
};

bool isSUVI(const Header& header);
#endif
