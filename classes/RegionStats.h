#pragma once
#ifndef RegionStats_H
#define RegionStats_H

#include <limits>
#include <cmath>
#include <iostream>
#include "constants.h"
#include "Region.h"
#include "Coordinate.h"
#include "EUVImage.h"
#include "ColorMap.h"
#include "FitsFile.h"

//! Class to obtain information about the statistics of a region
/*!
This class augments the Region information with statistical information about area, position error,
and common statistics mesure on the intensities of the region like mean, variance, etc.

*/

class RegionStats : public Region
{

	private :
		Real m1, m2, m3, m4, minIntensity, maxIntensity, totalIntensity, centerxError, centeryError, area_Raw, area_RawUncert, area_AtDiskCenter, area_AtDiskCenterUncert, numberContourPixels;
		Real barycenter_x, barycenter_y;
		std::vector<EUVPixelType> intensities;
	private :
		//! Routine to update a region with a new pixel
		void add(const Coordinate& pixelCoordinate, const EUVPixelType& pixelIntensity, const Coordinate sunCenter, const bool atBorder, const double R);
		//! Routine to compute the moments from the pixel intensities vector
		void computeMoments();

	public :
		//! Constructor
		RegionStats();
		//! Constructor
		RegionStats(const time_t& observationTime);
		//! Constructor
		RegionStats(const time_t& observationTime, const unsigned id, const ColorType color = 0);
		
		Real Mean() const;
		Real Variance() const;
		Real Skewness() const;
		Real Kurtosis() const;
		Real CenterxError() const;
		Real CenteryError() const;
		Real MinIntensity() const;
		Real MaxIntensity() const;
		Real TotalIntensity() const;
		//! Area of the region as seen on the image (Mm²)
		Real Area_Raw() const;
		//! Uncertainty of the Raw Area
		Real Area_RawUncert() const;
		//! Area of the region as it would be if the region was centered on the disk (Mm²)
		Real Area_AtDiskCenter() const;
		//! Uncertainty of the Area at Disk Center
		Real Area_AtDiskCenterUncert() const;
		//! The center of the region weighted by the pixel intensity
		Coordinate Barycenter() const;
		
		//! Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;



	public :
		//! Compute and returns all the region statistics of an image
		/* 
		@param colorizedComponentsMap A map of the region, each one must have a different color
		@param image The image to compute the intensities statistics.
		*/
		friend std::vector<RegionStats*> getRegionStats(const ColorMap* colorizedComponentsMap, const EUVImage* image);
		
		//! Write the regions into a fits file as column into the current table 
		friend FitsFile& writeRegions(FitsFile& file, const std::vector<RegionStats*>& regionStats);

};

std::vector<RegionStats*> getRegionStats(const ColorMap* colorizedComponentsMap, const EUVImage* image);
FitsFile& writeRegions(FitsFile& file, const std::vector<RegionStats*>& regionStats);

#endif
