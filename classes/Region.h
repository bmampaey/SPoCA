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
#include "SunImage.h"

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
		Region();
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
		std::string Label() const;
		

	public :

		static const std::string header;
		friend std::ostream& operator<<(std::ostream& out, const Region& r);
		friend std::vector<Region*> getRegions(const SunImage* colorizedComponentsMap);

};

std::vector<Region*> getRegions(const SunImage* colorizedComponentsMap);

#endif
