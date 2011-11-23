#pragma once
#ifndef HMIImage_H
#define HMIImage_H

#include "EUVImage.h"
#include "Header.h"

class HMIImage : public EUVImage
{
	public :
		//! Constructor
		HMIImage();
		
		//! Constructor for an HMIImage from an header
		HMIImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an HMIImage from a WCS
		HMIImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		HMIImage(const EUVImage& i);
		
		//! Copy Constructor
		HMIImage(const EUVImage* i);
		
		//! Destructors
		~HMIImage();
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Accessor to retrieve the Channel
		std::string Channel() const;
		
		//! Return a label for the image
		std::string Label() const;
		
		//! Routine that improves the contrast for visualization
		void enhance_contrast();
		
		//! Accessor to retrieve the Instrument
		std::string Instrument() const;

};

bool isHMI(const Header& header);
#endif

