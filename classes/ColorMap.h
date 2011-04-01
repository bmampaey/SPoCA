#pragma once
#ifndef ColorMap_H
#define ColorMap_H

#include "Header.h"
#include "SunImage.h"

class ColorMap : public SunImage<unsigned>
{
	public :
		//Constructors and destructors
		ColorMap(const long xAxes = 0, const long yAxes = 0);
		ColorMap(const SunImage<unsigned>& i);
		ColorMap(const SunImage<unsigned>* i);
		ColorMap(const Header& header);
		~ColorMap();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
		
		//Routines to treshold regions by size
		void tresholdRegionsByRawArea(const double minSize);
		void tresholdRegionsByRealArea(const double minSize);

};

bool isColorMap(const Header& header);
#endif
