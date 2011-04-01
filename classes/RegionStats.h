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

class RegionStats : public Region
{

	private :
		Real m1, m2, m3, m4, minIntensity, maxIntensity, totalIntensity, centerxError, centeryError, area_Raw, area_RawUncert, area_AtDiskCenter, area_AtDiskCenterUncert, numberContourPixels;
		Real barycenter_x, barycenter_y;
		std::vector<PixelType> intensities;
	private :
		//Update routines
		void add(const Coordinate& pixelCoordinate, const PixelType& pixelIntensity, const Coordinate sunCenter, const bool atBorder, const double R);
		void computeMoments();

	public :
		//Constructors
		RegionStats();
		RegionStats(const time_t& observationTime);
		RegionStats(const time_t& observationTime, const unsigned id, const unsigned long color = 0);
		
		// Accessors
		Real Mean() const;
		Real Variance() const;
		Real Skewness() const;
		Real Kurtosis() const;
		Real CenterxError() const;
		Real CenteryError() const;
		Real MinIntensity() const;
		Real MaxIntensity() const;
		Real TotalIntensity() const;
		Real Area_Raw() const;
		Real Area_RawUncert() const;
		Real Area_AtDiskCenter() const;
		Real Area_AtDiskCenterUncert() const;
		Coordinate Barycenter() const;
				
		// Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;



	public :
		friend std::vector<RegionStats*> getRegionStats(const ColorMap* colorizedComponentsMap, const EUVImage* image);
		friend FitsFile& writeRegions(FitsFile& file, const std::vector<RegionStats*>& regionStats);

};

std::vector<RegionStats*> getRegionStats(const ColorMap* colorizedComponentsMap, const EUVImage* image);
FitsFile& writeRegions(FitsFile& file, const std::vector<RegionStats*>& regionStats);

#endif
