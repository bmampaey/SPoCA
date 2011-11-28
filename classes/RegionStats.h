#pragma once
#ifndef RegionStats_H
#define RegionStats_H

#include <limits>
#include <cmath>
#include <deque>

#include "constants.h"
#include "tools.h"
#include "Coordinate.h"
#include "EUVImage.h"
#include "ColorMap.h"
#include "FitsFile.h"
#include "Region.h"
#include "SegmentationStats.h"

//! Class to obtain information about the statistics of a region
/*!
This class gives statistics information about a Region such as center, center error, area,
and common statistics mesure on the intensities of the region like mean, variance, etc.

*/

class RegionStats
{

	private :
		//! Unique and invariable identifier for a region at time observationTime
		unsigned id;
		//! Observation Time for the stats
		time_t observationTime;
		//! Total number of pixels in the region
		unsigned numberPixels;
		// Moments
		mutable Real m2, m3, m4;
		Real minIntensity, maxIntensity, totalIntensity, centerxError, centeryError, area_Raw, area_RawUncert, area_AtDiskCenter, area_AtDiskCenterUncert, numberContourPixels;
		//! Coordinates of the center of the region
		RealPixLoc center, barycenter;
		bool clipped_spatial;
		mutable std::deque<EUVPixelType> intensities;
	private :
		//! Routine to compute the moments from the pixel intensities vector
		void computeMoments();

	public :
		//! Constructor
		RegionStats(const time_t& observationTime, const unsigned id = 0);
		
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
		//! Mean of the intensities the region
		Real Mean() const;
		//! Median of the intensities the region
		Real Median() const;
		//! Variance of the intensities the region
		Real Variance() const;
		//! Skewness of the intensities the region
		Real Skewness() const;
		//! Kurtosis of the intensities the region
		Real Kurtosis() const;
		//! The geometric center of the region
		RealPixLoc Center() const;
		//! The center of the region weighted by the pixel intensity
		RealPixLoc Barycenter() const;
		//! Error for the center x coordinate in pixels
		Real CenterxError() const;
		//! Error for the center y coordinate in pixels
		Real CenteryError() const;
		//! Minimum intensity of the region
		Real MinIntensity() const;
		//! Maximum intensity of the region
		Real MaxIntensity() const;
		//! Total intensity of the region
		Real TotalIntensity() const;
		//! Area of the region as seen on the image (Mm²)
		Real Area_Raw() const;
		//! Uncertainty of the Raw Area
		Real Area_RawUncert() const;
		//! Area of the region as it would be if the region was centered on the disk (Mm²)
		Real Area_AtDiskCenter() const;
		//! Uncertainty of the Area at Disk Center
		Real Area_AtDiskCenterUncert() const;
		//! If the region is clipped spatially, i.e. some of it's pixels are neer the limb
		bool ClippedSpatial() const;
		
		//! Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;

		//! Routine to update a region with a new pixel
		void add(const PixLoc& coordinate, const EUVPixelType& pixelIntensity, const RealPixLoc& sunCenter, const bool& atBorder, const Real& R);
};

//! Compute all statistics of an image using a ColorMap as a cache
/* 
@param map A map of the region, each one must have a different color
@param image The image to compute the intensities statistics.
*/
std::vector<RegionStats*> getRegionStats(const ColorMap* coloredMap, const EUVImage* image);

//! Compute the statistics of the regions using a ColorMap as a cache
/* 
@param map A map of the region, each one must have a different color
@param image The image to compute the intensities statistics.
@param regions The regions for wich to compute the stats
*/
std::vector<RegionStats*> getRegionStats(const ColorMap* coloredMap, const EUVImage* image, const std::vector<Region*>& regions);

//! Compute the statistics of all the regions taken together using a ColorMap as a cache
/* 
@param map A map of the region, each one must have a color > 0
@param image The image to compute the intensities statistics.
@return A vector of 2 SegmentationStats, the one with id 1 is the stats of all regions, the one with id 0 is the stats of the complement
*/
std::vector<SegmentationStats*> getTotalRegionStats(const ColorMap* coloredMap, const EUVImage* image);

//! Write the regions into a fits file as column into the current table 
FitsFile& writeRegions(FitsFile& file, const std::vector<RegionStats*>& regions_stats);

#endif
