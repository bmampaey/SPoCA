#pragma once
#ifndef EUVImage_H
#define EUVImage_H

#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>

#include "Coordinate.h"
#include "SunImage.h"

#ifdef MAGICK
#include "MagickImage.h"
#endif


class EUVImage : public SunImage<EUVPixelType>
{

	protected :
		//! wavelength of the EUV image in Angstrom
		Real wavelength;
		//! median of the EUV image
		Real median;
		//! mode of the EUV image
		Real mode;
		//! percentile of the EUV image
		Real datap01;
		//! percentile of the EUV image
		Real datap95;
		//! exposure time of the EUV image
		Real exposureTime;
		
		//! Parameters fo the Annulus Limb Correction
		std::vector<Real> ALCParameters;
	
		//! Routine to return the radius of the disc not corrected by the ALC
		virtual Real MINRADIUS()
		{ return ALCParameters[0]; }

	public :
		//! Constructor for an EUVImage of size xAxes x yAxes
		EUVImage(const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Constructor for an EUVImage of size xAxes x yAxes, with sun center and radius
		EUVImage(const unsigned& xAxes, const unsigned& yAxes, const RealPixLoc& suncenter, const Real& sunradius);
		
		//! Constructor for an EUVImage from an header
		EUVImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an EUVImage from a WCS
		EUVImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		EUVImage(const EUVImage& i);
		
		//! Copy Constructor
		EUVImage(const EUVImage* i);

		//!Destructor
		~EUVImage();
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Accessor to retrieve the Channel
		virtual std::string Channel() const;
		
		//! Return a label for the image
		std::string Label() const;
		
		//! Accessor to retrieve the Wavelength in Angstrom
		Real Wavelength() const;
		
		//! Accessor to retrieve the median value
		Real Median() const;
		
		//! Accessor to retrieve the mode
		Real Mode() const;
		
		//! Accessor to retrieve the exposure time in seconds
		Real ExposureTime() const;
		
		//! Routine to get the ALC parameters
		virtual std::vector<Real> getALCParameters();
		
		//! Routine to set the ALC parameters
		virtual void setALCParameters(std::vector<Real> ALCParameters);

		//! Routine to do image preprocessing
		void preprocessing(std::string preprocessingList, const Real radiusRatio = 1.0);
		
		//! Routine to do Annulus Limb Correction (ALC)
		void annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius);
		
		//! Routine to do Annulus Limb Correction followed by a division by the median of the inner disc
		void ALCDivMedian(Real maxLimbRadius, Real minLimbRadius);
		
		//! Function that gives the percententage of necessary correction for the Annulus Limb Correction
		Real percentCorrection(const Real r) const;
		
		//! Routine that improves the contrast for visualization
		/*! Does a simple thresholding of the pixel intensities on the percentiles 1% and 95% */ 
		virtual void enhance_contrast();
		
		//! Routine that returns the color table
		/*! Based on the routine aia_lct.pro from solar soft */
		virtual std::vector<char> color_table() const;
		
		#ifdef MAGICK
		//! Routine that creates and return a grey MagickImage 
		MagickImage magick();
		#endif

};

#endif
