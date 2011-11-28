#pragma once
#ifndef WCS_H
#define WCS_H

#include <ctime>
#include <string>


#include "constants.h"
#include "Coordinate.h"
#include "FitsFile.h"


class WCS
{
	public:
		
		//! Location of the center of the sun in the image
		RealPixLoc sun_center;

		//! The radius of the sun in pixels
		Real sun_radius;
		
		//! Date & Time of observation as a string
		std::string date_obs;
		
		//! Length of pixel in arcsec
		Real cdelt1;
		
		//! Width of pixel in arcsec
		Real cdelt2;
		
		//! Heliographic Latitude of the observer/satellite in radians
		Real b0;
		
		//! Heliographic Longitude of the observer/satellite in radians
		Real l0;
		
		//! Carrington longitude of the observer/satellite
		Real carrington_l0;
		
		//! Distance between observer/satellite and sun in Mmeters
		double dsun_obs;
		
		//! Radius of the sun in Mmeters
		double sunradius_Mm;

		//! Time of observation
		time_t time_obs;
		
		//! Rotation Matrix of the roll angle of the observer/satellite
		Real cd[2][2];
		
		//! Inverse Rotation Matrix of the roll angle of the observer/satellite
		Real icd[2][2];
		
		//! Cosine of b0 for faster computation
		Real cos_b0;

		//! Sine of b0 for faster computation
		Real sin_b0;
		
		//! Constructor
		WCS();
		
		//! Constructor
		WCS(const RealPixLoc& sun_center, const Real& sun_radius = 0);
		
		void setSunCenter(const Real& crpix1, const Real& crpix2);
		void setSunradius(const double& sun_radius);
		void setDateObs(std::string date_obs);
		void setCDelt(const Real& cdelt1, const Real& cdelt2);
		void setB0(const Real& b0);
		void setL0(const Real& l0);
		void setCarringtonL0(const Real& l0);
		void setDistanceSunObs(const double& dsun_obs);
		//! Set the CD matrix from a CROTA2
		void setCrota2(const Real& crota2);
		//! Set the CD matrix from a PC matrix
		void setPC(const Real& pc1_1, const Real& pc1_2, const Real& pc2_1, const Real& pc2_2);
		//! Set the CD matrix and the inverse CD matrix
		void setCD(const Real& cd1_1, const Real& cd1_2, const Real& cd2_1, const Real& cd2_2);
};

#endif
