#pragma once
#ifndef TrackingEdge_H
#define TrackingEdge_H

#include <string>
#include <vector>

#include "constants.h"
#include "TrackingRegion.h"
#include "ColorMap.h"

class TrackingEdge
{
	public:
		std::string origin;
		std::string destination;
		unsigned intersectNumberPixels;
		Real intersectArea_arcsec2;
		Real intersectArea_Mm2;
	
	public:
		TrackingEdge();
		TrackingEdge(const std::string& origin, const std::string& destination, const unsigned& intersectNumberPixels = 0, const Real& intersectArea_arcsec2 = 0, const Real& intersectArea_Mm2 = 0);
		std::string toJSON() const;
};

//! Return a new TrackingEdge if regions overlap, NULL otherwise
TrackingEdge* get_edge(const ColorMap* image1, const TrackingRegion* region1 , const ColorMap* image2, const TrackingRegion* region2);

//! Return a new TrackingEdge if regions overlap with derotation, NULL otherwise
TrackingEdge* get_edge_derotate(const ColorMap* image1, const TrackingRegion* region1 , const ColorMap* image2, const TrackingRegion* region2);

//! Return a list of TrackingEdge between the regions1 and regions 2
std::vector<TrackingEdge*> get_edges(const ColorMap* image1, const std::vector<TrackingRegion*>& regions1 , const ColorMap* image2, const std::vector<TrackingRegion*>& regions2, bool derotate = true);
#endif
