#include "TrackingEdge.h"

using namespace std;

TrackingEdge::TrackingEdge()
{
	intersectNumberPixels = 0;
	intersectArea_arcsec2 = 0;
	intersectArea_Mm2 = 0;
}

TrackingEdge::TrackingEdge(const string& origin, const string& destination, const unsigned& intersectNumberPixels, const Real& intersectArea_arcsec2, const Real& intersectArea_Mm2)
{
	this->origin = origin;
	this->destination = destination;
	this->intersectNumberPixels = intersectNumberPixels;
	this->intersectArea_arcsec2 = intersectArea_arcsec2;
	this->intersectArea_Mm2 = intersectArea_Mm2;
}

string TrackingEdge::toJSON() const
{
	ostringstream out;
	out<<"{\n";
	out<<"\"origin\": \""<<origin<<"\",\n";
	out<<"\"destination\": \""<<destination<<"\",\n";
	out<<"\"NumberPixels\" :"<<intersectNumberPixels<<",\n";
	out<<"\"Area_arcsec2\" :"<<intersectArea_arcsec2<<",\n";
	out<<"\"Area_Mm2\" :"<<intersectArea_Mm2<<",\n";
	out<<"}";
	return out.str();
}

TrackingEdge* get_edge(const ColorMap* image1, const TrackingRegion* region1 , const ColorMap* image2, const TrackingRegion* region2)
{
	ColorType setValue1 = image1->pixel(region1->FirstPixel());
	ColorType setValue2 = image2->pixel(region2->FirstPixel());
	
	PixLoc r1_boxmin = region1->Boxmin();
	PixLoc r1_boxmax = region1->Boxmax();
	PixLoc r2_boxmin = region2->Boxmin();
	PixLoc r2_boxmax = region2->Boxmax();

	unsigned Xmin = r1_boxmin.x > r2_boxmin.x ? r1_boxmin.x : r2_boxmin.x;
	unsigned Ymin = r1_boxmin.y > r2_boxmin.y ? r1_boxmin.y : r2_boxmin.y;
	unsigned Xmax = r1_boxmax.x < r2_boxmax.x ? r1_boxmax.x : r2_boxmax.x;
	unsigned Ymax = r1_boxmax.y < r2_boxmax.y ? r1_boxmax.y : r2_boxmax.y;
	
	Real intersectArea_arcsec2 = 0;
	Real intersectArea_Mm2 = 0;
	unsigned intersectPixels = 0;
	
	// We scan the intersection between the 2 boxes of the regions
	// If the 2 regions don't overlay, we will not even enter the loops
	PixLoc c;
	for (c.y = Ymin; c.y <= Ymax; ++c.y)
	{
		for (c.x = Xmin; c.x <= Xmax; ++c.x)
		{
			// We check if there is overlay between the two regions
			if(image1->pixel(c) == setValue1 && image2->pixel(c) == setValue2)
			{
				++intersectPixels;
				intersectArea_arcsec2 += image2->PixelArea();
				intersectArea_Mm2 += image2->RealPixelArea(c);
			}
		}
	}
	if (intersectPixels > 0)
	{
		return new TrackingEdge(region1->HekLabel(), region2->HekLabel(), intersectPixels, intersectArea_arcsec2, intersectArea_Mm2);
	}
	else
	{
		return NULL;
	}
}

TrackingEdge* get_edge_derotate(const ColorMap* image1, const TrackingRegion* region1 , const ColorMap* image2, const TrackingRegion* region2)
{
	ColorType setValue1 = image1->pixel(region1->FirstPixel());
	ColorType setValue2 = image2->pixel(region2->FirstPixel());
	
	
	// We are going to project the image1 into the coordinate of image2
	RealPixLoc r1_boxmin = image1->shift_like(region1->Boxmin(), image2);
	RealPixLoc r1_boxmax = image1->shift_like(region1->Boxmax(), image2);
	PixLoc r2_boxmin = region2->Boxmin();
	PixLoc r2_boxmax = region2->Boxmax();

	//The projection of the box of region1 may lie outside of the sundisc ==> the projection is null 
	if(!r1_boxmin)
		 r1_boxmin = r2_boxmin;
	if(!r1_boxmax)
		 r1_boxmax = r2_boxmax;
	
	Real intersectArea_arcsec2 = 0;
	Real intersectArea_Mm2 = 0;
	unsigned intersectPixels = 0;
	
	// We compute the bornes of the intersection of the 2 regions
	// If the 2 regions don't overlay, we will not even enter the loops
	unsigned Xmin = unsigned(r1_boxmin.x > r2_boxmin.x ? r1_boxmin.x : r2_boxmin.x);
	unsigned Ymin = unsigned(r1_boxmin.y > r2_boxmin.y ? r1_boxmin.y : r2_boxmin.y);
	unsigned Xmax = unsigned(r1_boxmax.x < r2_boxmax.x ? r1_boxmax.x : r2_boxmax.x);
	unsigned Ymax = unsigned(r1_boxmax.y < r2_boxmax.y ? r1_boxmax.y : r2_boxmax.y);

	// We scan the intersection in the coordinates of image2
	PixLoc c2;
	for (c2.y = Ymin; c2.y <= Ymax; ++c2.y)
	{
		for (c2.x = Xmin; c2.x <= Xmax; ++c2.x)
		{
			// We project back the coordinate of image2 into the coordinate of image1
			RealPixLoc  c1 = image2->shift_like(c2, image1);
			// The projection of the coordinate may lie outside of the sundisc ==> the projection is null
			if (!c1)
				continue;
			// We check if there is overlay between the two regions 
			if(image1->interpolate(c1) == setValue1 && image2->pixel(c2) == setValue2)
			{
				++intersectPixels;
				intersectArea_arcsec2 += image2->PixelArea();
				intersectArea_Mm2 += image2->RealPixelArea(c2);
			}
		}
	}
	
	if (intersectPixels > 0)
	{
		return new TrackingEdge(region1->HekLabel(), region2->HekLabel(), intersectPixels, intersectArea_arcsec2, intersectArea_Mm2);
	}
	else
	{
		return NULL;
	}
}

vector<TrackingEdge*> get_edges(const ColorMap* image1, const vector<TrackingRegion*>& regions1 , const ColorMap* image2, const vector<TrackingRegion*>& regions2, bool derotate)
{
	vector<TrackingEdge*> edges;
	for (unsigned r1 = 0; r1 < regions1.size(); ++r1)
	{
		for (unsigned r2 = 0; r2 < regions2.size(); ++r2)
		{
			TrackingEdge* edge;
			if (derotate)
				edge = get_edge_derotate(image1, regions1[r1], image2, regions2[r2]);
			else
				edge = get_edge(image1, regions1[r1], image2, regions2[r2]);
			if (edge != NULL)
				edges.push_back(edge);
		}
	}
	return edges;
}


