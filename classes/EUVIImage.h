#pragma once
#ifndef EUVIImage_H
#define EUVIImage_H

#include "EUVImage.h"
#include "Header.h"

class EUVIImage : public EUVImage
{
	public :
		//! Constructor
		EUVIImage();
		
		//! Constructor for an EUVIImage from an header
		EUVIImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an EUVIImage from a WCS
		EUVIImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		EUVIImage(const EUVImage& i);
		
		//! Copy Constructor
		EUVIImage(const EUVImage* i);
		
		//! Destructors
		~EUVIImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Accessor to retrieve the Instrument
		std::string Instrument() const;
		
		//! Routine that returns the color table
		std::vector<char> color_table() const;
};

bool isEUVI(const Header& header);
#endif
