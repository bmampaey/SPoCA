#pragma once
#ifndef TrackingRegion_H
#define TrackingRegion_H

#include <limits>
#include <cmath>
#include <deque>
#include <string>

#include "constants.h"
#include "tools.h"
#include "Coordinate.h"
#include "ColorMap.h"
#include "FitsFile.h"

//! Class to obtain information about the statistics of a region
/*!
This class compute informations about a Region necessary for tracking
*/

class TrackingRegion
{

	private :
		//! Unique and invariable identifier for a region at time observationTime
		unsigned id;
		//! Observation Time for the stats
		time_t observationTime;
		//! Total number of pixels in the region
		unsigned numberPixels;
		// Moments
		Real area_arcsec2, area_Mm2;
		//! Coordinates of the center of the region
		RealPixLoc center;
		//! Helio graphic coordinates of the center of the region
		HGS HGcenter ;
		//! Coordonates of the first (lower left) pixel of the region
		PixLoc first;
		//! Coordonates of the lower left corner of the box surrounding the region
		PixLoc boxmin;
		//! Coordonates of the upper right corner of the box surrounding the region
		PixLoc boxmax;

	public :
		//! Constructor
		TrackingRegion(const time_t& observationTime, const unsigned id = 0);
		
		//! Accessor to retrieve the id
		unsigned Id() const;

		//! Accessor to set the id
		void setId(const unsigned& id);
		
		//! Accessor to retrieve the observation time
		time_t ObservationTime() const;
		
		//! Accessor to retrieve the observation time as a string
		std::string ObservationDate() const;
		
		//! Accessor to retrieve the number of pixels
		unsigned NumberPixels() const;
		
		//! Area of the region as seen on the image (arcsec²)
		Real Area_arcsec2() const;
		
		//! Area of the region as it would be if the region was centered on the disk (Mm²)
		Real Area_Mm2() const;
		
		//! The geometric center of the region
		RealPixLoc Center() const;
		
		//! The real center of the region
		HGS HGCenter() const;
		
		//! Routine that returns the very first pixel of the region.
		PixLoc FirstPixel() const;
		
		//! Routine that returns the lower left corner coordinate of the box surrounding the region
		PixLoc Boxmin() const;
		
		//! Routine that returns the upper right corner coordinate of the box surrounding the region
		PixLoc Boxmax() const;
		
		//! Routine that returns a label for the HEK.
		std::string HekLabel() const;
		
		//! Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;

		//! Output a region as a JSON string
		std::string toJSON() const;

		//! Routine to update a region with a new pixel
		void add(const PixLoc& coordinate, const HGS& longlat, const Real& pixel_area_arcsec2, const Real& pixel_area_Mm2);
};

//! Compute the information about the regions in a ColorMap
/* 
@param map A map of the region, each one must have a different color
*/
std::vector<TrackingRegion*> getTrackingRegions(const ColorMap* coloredMap);


//! Write the regions into a fits file as column into the current table 
FitsFile& writeRegions(FitsFile& file, const std::vector<TrackingRegion*>& regions);

#endif
