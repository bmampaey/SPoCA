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
		double wavelength;
		//! median of the EUV image
		double median;
		//! mode of the EUV image
		double mode;
		//! percentile of the EUV image
		double datap01;
		//! percentile of the EUV image
		double datap95;
		//! exposure time of the EUV image
		double exposureTime;
		
		//! Parameters fo the Annulus Limb Correction
		std::vector<Real> ALCParameters;
	
		//! Routine to return the radius of the disc not corrected by the ALC
		virtual Real MINRADIUS()
		{ return ALCParameters[0]; }

	public :
		
		//!Constructor
		EUVImage(const long xAxes = 0, const long yAxes = 0);
		//!Constructor
		EUVImage(const long xAxes, const long yAxes, const Coordinate suncenter, const double radius);
		//!Constructor
		EUVImage(const EUVImage& i);
		//!Constructor
		EUVImage(const EUVImage* i);
		//!Constructor
		EUVImage(const Header& header);
		
		//!Destructor
		~EUVImage();
		
		//! Routine to read the sun parameters from the header
		void postRead();
		
		//! Routine to write the sun parameters to the header
		void preWrite();
		
		//! Accessor to retrieve the Wavelength in Angstrom
		double Wavelength() const;
		//! Accessor to retrieve the median value
		double Median() const;
		//! Accessor to retrieve the exposure time in seconds
		double ExposureTime() const;
		
		//! Routine to copy the parameters from another EUVImage
		void copySunParameters(const EUVImage* i);
		
		//! Routine to set the ALC parameters
		void setALCParameters(const std::vector<Real>& ALCParameters);
		
		//! Routine to get the ALC parameters
		std::vector<Real> getALCParameters();

		//! Routine to do image preprocessing
		void preprocessing(std::string preprocessingList, const Real radiusRatio = 1.0);
		//! Routine to do Annulus Limb Correction (ALC)
		void annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius);
		//! Routine to do Annulus Limb Correction followed by a division by the median of the inner disc
		void ALCDivMedian(Real maxLimbRadius, Real minLimbRadius);
		
		//! Routine to do Annulus Limb Correction followed by a division by the mode of the inner disc
		void ALCDivMode(Real maxLimbRadius, Real minLimbRadius);
		
		//! Function that gives the percententage of necessary correction for the Annulus Limb Correction
		Real percentCorrection(const Real r) const;
		
		#ifdef MAGICK
		//! Routine that creates and return a grey MagickImage 
		MagickImage magick();
		#endif

};

#endif
