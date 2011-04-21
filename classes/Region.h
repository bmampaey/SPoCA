#pragma once
#ifndef Region_H
#define Region_H

#include <limits>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>

#include "constants.h"
#include "Coordinate.h"
#include "ColorMap.h"
#include "FitsFile.h"

//! Class to obtain information about the position in space and time of a region

class Region
{
	protected :
		unsigned  id;
		time_t observationTime;
		unsigned long color;
		Coordinate first, boxmin, boxmax, center;
		unsigned numberPixels;
		
	protected :
		//! Routine to update a region with a new pixel
		void add(const unsigned& x, const unsigned& y);
		//! Routine to update a region with a new pixel
		void add(const Coordinate& pixelCoordinate);

	public :
		//! Constructor
		Region(const unsigned id = 0);
		//! Constructor
		Region(const time_t& observationTime);
		//! Constructor
		Region(const time_t& observationTime, const unsigned id, const ColorType color = 0);

		bool operator==(const Region& r)const;
		unsigned  Id() const;
		void setId(const unsigned& id);
		ColorType Color() const;
		void setColor(const ColorType& color);
		//! Routine that returns the lower left corner coordinate of the box surrounding the region
		Coordinate Boxmin() const;
		//! Routine that returns the upper right corner coordinate of the box surrounding the region
		Coordinate Boxmax() const;
		//! Routine that returns the geometric center of the region
		Coordinate Center() const;
		//! Routine that returns the very first pixel of the region.
		Coordinate FirstPixel() const;
		unsigned NumberPixels() const;
		time_t ObservationTime() const;
		std::string ObservationDate() const;
		//! Routine that returns a label for the HEK.
		std::string HekLabel() const;
		std::string Visu3DLabel() const;
		//! Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;

	public :
		//! Compute and returns all the regions image
		/* 
		@param colorizedComponentsMap A map of the region, each one must have a different color
		*/
		friend std::vector<Region*> getRegions(const ColorMap* colorizedComponentsMap);
		//! Write the regions into a fits file as column into the current table 
		friend FitsFile& writeRegions(FitsFile& file, const std::vector<Region*>& regions);
		//! Read the regions from the fits file current table
		friend FitsFile& readRegions(FitsFile& file, std::vector<Region*>& regions);

};

std::vector<Region*> getRegions(const ColorMap* colorizedComponentsMap);
FitsFile& writeRegions(FitsFile& file, const std::vector<Region*>& regions);
FitsFile& readRegions(FitsFile& file, std::vector<Region*>& regions);
#endif

