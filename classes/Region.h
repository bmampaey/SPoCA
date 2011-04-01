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


class Region
{
	protected :
		unsigned  id;
		time_t observationTime;
		unsigned long color;
		Coordinate first, boxmin, boxmax, center;
		unsigned numberPixels;
		
	protected :
		//Update routines
		void add(const unsigned& x, const unsigned& y);
		void add(const Coordinate& pixelCoordinate);

	public :
		//Constructors
		Region(const unsigned id = 0);
		Region(const time_t& observationTime);
		Region(const time_t& observationTime, const unsigned id, const unsigned long color = 0);

		//accessor and operators
		bool operator==(const Region& r)const;
		unsigned  Id() const;
		void setId(const unsigned& id);
		unsigned long Color() const;
		void setColor(const unsigned long& color);
		Coordinate Boxmin() const;
		Coordinate Boxmax() const;
		Coordinate Center() const;
		Coordinate FirstPixel() const;
		unsigned NumberPixels() const;
		time_t ObservationTime() const;
		std::string ObservationDate() const;
		std::string HekLabel() const;
		std::string Visu3DLabel() const;
		// Output a region as a string
		std::string toString(const std::string& separator, bool header = false) const;

	public :
		friend std::vector<Region*> getRegions(const ColorMap* colorizedComponentsMap);
		friend FitsFile& writeRegions(FitsFile& file, const std::vector<Region*>& regions);
		friend FitsFile& readRegions(FitsFile& file, std::vector<Region*>& regions);

};

std::vector<Region*> getRegions(const ColorMap* colorizedComponentsMap);
FitsFile& writeRegions(FitsFile& file, const std::vector<Region*>& regions);
FitsFile& readRegions(FitsFile& file, std::vector<Region*>& regions);
#endif

