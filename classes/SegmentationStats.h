#pragma once
#ifndef SegmentationStats_H
#define SegmentationStats_H

#include <limits>
#include <cmath>
#include <deque>

#include "constants.h"
#include "tools.h"
#include "Coordinate.h"
#include "EUVImage.h"
#include "ColorMap.h"
#include "FitsFile.h"

//! Class to obtain information about the statistics of a class
/*!
This class gives statistics information about Segmentation classes such as area, filling factor
and common statistics mesure on the intensities of the class like mean, variance, etc.

*/

class SegmentationStats
{

	private :
		//! Class id
		unsigned id;
		//! Observation Time for the stats
		time_t observationTime;
		//! Total number of pixels in the class
		unsigned numberPixels;
		//! Moments
		mutable Real m2, m3, m4;
		Real minIntensity, maxIntensity, totalIntensity, area_Raw, area_AtDiskCenter, fillingFactor;
		mutable std::deque<EUVPixelType> intensities;
	private :
		//! Routine to compute the moments from the pixel intensities vector
		void computeMoments();

	public :
		//! Constructor
		SegmentationStats(const time_t& observationTime, const unsigned id = 0);
		
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
		//! Mean of the intensities the class
		Real Mean() const;
		//! Median of the intensities the class
		Real Median() const;
		//! Variance of the intensities the class
		Real Variance() const;
		//! Skewness of the intensities the class
		Real Skewness() const;
		//! Kurtosis of the intensities the class
		Real Kurtosis() const;
		//! Minimum intensity of the class
		Real MinIntensity() const;
		//! Maximum intensity of the class
		Real MaxIntensity() const;
		//! Total intensity of the class
		Real TotalIntensity() const;
		//! Area of the class as seen on the image (Mm²)
		Real Area_Raw() const;
		//! Area of the class as it would be if the class was centered on the disk (Mm²)
		Real Area_AtDiskCenter() const;
		//! Filling factor of the class
		Real FillingFactor() const;
		//! Output a class as a string
		std::string toString(const std::string& separator, bool header = false) const;

		//! Routine to update a class with a new pixel
		void add(const PixLoc& coordinate, const EUVPixelType& pixelIntensity, const RealPixLoc& sunCenter, const Real& R);
};

//! Compute all statistics of an image using a ColorMap as a cache
/* 
@param map A segmentation map, each class must have a different color
@param image The image to compute the intensities statistics.
*/
std::vector<SegmentationStats*> getSegmentationStats(const ColorMap* coloredMap, const EUVImage* image);

//! Compute the statistics of the classes using a ColorMap as a cache
/* 
@param map A segmentation map, each class must have a different color
@param image The image to compute the intensities statistics.
@param classes The classes for wich to compute the stats
*/
std::vector<SegmentationStats*> getSegmentationStats(const ColorMap* coloredMap, const EUVImage* image, const std::vector<ColorType>& classes);

//! Write the classes into a fits file as column into the current table 
FitsFile& writeRegions(FitsFile& file, const std::vector<SegmentationStats*>& segmentation_stats);

#endif
