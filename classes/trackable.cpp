#include "trackable.h"
#include <map>

using namespace std;


ColorType newColor;

// Compute the number of pixels common to 2 regions from 2 images, with derotation
unsigned overlay_derotate(ColorMap* image1, const Region* region1, ColorMap* image2, const Region* region2)
{
	unsigned intersectPixels = 0;
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
				++intersectPixels;
		}
	}
	
	return intersectPixels;

}


// Compute the number of pixels common to 2 regions from 2 images
unsigned overlay(ColorMap* image1, const Region* region1, ColorMap* image2, const Region* region2)
{
	unsigned intersectPixels = 0;
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

	// We scan the intersection between the 2 boxes of the regions
	// If the 2 regions don't overlay, we will not even enter the loops
	for (unsigned y = Ymin; y <= Ymax; ++y)
	{
		for (unsigned x = Xmin; x <= Xmax; ++x)
		{
			// We check if there is overlay between the two regions
			if(image1->pixel(x,y) == setValue1 && image2->pixel(x,y) == setValue2)
				++intersectPixels;
		}
	}
	
	return intersectPixels;

}

// Color a node
void RegionGraph::node::colorize()
{
	//If I'm already colored than I am fine
	if(region->Color() != 0)
		return;

	// I need all my parents to have their color
	RegionGraph::node::const_iterator biggestInEdge = in_begin();
	for (RegionGraph::node::const_iterator it = in_begin(); it != in_end(); ++it)
	{
		//Carefull there is recursion here
		it->from->colorize();
		// We search for the biggest parent
		if(it->weight > biggestInEdge->weight)
			biggestInEdge = it;
		// We inherit the firstObservationTime of my parents
		if(it->from->get_region()->FirstObservationTime() < region->FirstObservationTime())
			region->setFirstObservationTime(it->from->get_region()->FirstObservationTime());

	}
	//Either I am the only child of my biggest parent, or I am his biggest Son
	if(biggestInEdge != in_end() && region == biggestInEdge->from->biggestSon()->get_region())
	{
		region->setColor(biggestInEdge->from->get_region()->Color());
		region->setFirstObservationTime(biggestInEdge->from->get_region()->FirstObservationTime());
	}
	//There was a split or a merge, or I have no parents
	else
		region->setColor(++newColor);
}


// Tell if there is a path between a node and a region
bool RegionGraph::node::path(const RegionGraph::node* to, std::set<node*>* visited)
{
	if(visited->find(this) != visited->end()) {
		return false;
	}
	
	visited->insert(this);

	if (to->get_region() == region) {
		return true;
	}

	for (std::vector<edge>::const_iterator it = out_edges.begin(); it != out_edges.end(); ++it)
	{
		if(it->to->path(to, visited))
			return true;

	}
	return false;
}


// Output a graph in the dot format
void ouputGraph(const RegionGraph& g, const vector<vector<Region*> >& regions, const string graphName, bool isColored)
{

	ofstream graphFile((filenamePrefix + graphName + ".dot").c_str());
	if (graphFile.good())
	{

		graphFile<<"digraph "<<graphName<<" {"<<endl;
		graphFile<<"node [penwidth=4];"<<endl;
		//First we output all node info
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			string rank;
			for (unsigned r = 0; r < regions[s].size(); ++r)
			{
				rank += " \"" + regions[s][r]->HekLabel() + "\"";
				graphFile <<"\""<< regions[s][r]->HekLabel()<<"\"";
				if(isColored && regions[s][r]->Color() != 0)
				{
					graphFile<<"[color=\""<<gradient[regions[s][r]->Color() % gradientMax]<<"\"];"<<endl;
				}
			}
			graphFile<<"{ rank=same; "<< rank <<" };" << endl;

		}
		// Then we output all edges info
		for (RegionGraph::const_iterator it_node = g.begin(); it_node != g.end(); ++it_node)
		{
			for (RegionGraph::node::const_iterator it_edge = it_node->out_begin(); it_edge != it_node->out_end(); ++it_edge)
			{
				graphFile <<"\""<< it_node->get_region()->HekLabel()<<"\" -> \"" << it_edge->to->get_region()->HekLabel()<< "\" [label="<<it_edge->weight<<"];" << endl;
			}

		}
		graphFile<<"}"<<endl;
	}
	graphFile.close();
}


// Output regions in the region format
void ouputRegions(const vector<vector<Region*> >& regions, string filename)
{
	ofstream regionFile((filename).c_str());
	if (regionFile.good())
	{
		if(regions.size() > 0 && regions[0].size() > 0)
			regionFile<<regions[0][0]->toString("|", true)<<endl<<endl;
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			for (unsigned r = 0; r < regions[s].size(); ++r)
			{
				regionFile<<regions[s][r]->toString("|")<<endl;
			}
			regionFile<<endl;
		}
	}
	regionFile.close();
}

// Comparison of 2 images according to time (for sorting)
inline bool compare(const ColorMap* a, const ColorMap* b)
{
	return a->ObservationTime() < b->ObservationTime();
}

//Ordonate the images according to time
inline void ordonate(vector<ColorMap*>& images)
{
	sort(images.begin(), images.end(), compare);

	#if defined EXTRA_SAFE
	//We verify that 2 images do not have the exact same time
	vector<ColorMap*>::iterator s1 = images.begin();
	vector<ColorMap*>::iterator s2 = images.begin() + 1;
	while (s2 != images.end())
	{
		if (unsigned(difftime((*s2)->ObservationTime(),(*s1)->ObservationTime())) == 0)
		{
			cerr<<"Error : 2 images have exactly the same time."<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			++s1;
		}
		s2 = s1 + 1;
	}
	#endif
}

// Return a vector of indices of the images vector ordonated according to time
inline vector<unsigned> imageOrder(const vector<ColorMap*>& images)
{
	vector<unsigned> indices(images.size(),0);
	vector<ColorMap*> images_copy = images;
	ordonate(images_copy);
	for(unsigned i = 0; i < images_copy.size(); ++i)
	{
		for(unsigned j = 0; j < images.size(); ++j)
		{
			if(images_copy[i] == images[j])
			{
				indices[i] = j;
				break;
			}
		}
	}
	return indices;
}

void recolorFromRegions(ColorMap* image, const vector<Region*>& regions)
{
	map<ColorType,ColorType> colorTransfo;
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		colorTransfo[image->pixel(regions[r]->FirstPixel())] = regions[r]->Color();
	}
	for (unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		if(image->pixel(j) != image->null())
		{
			#if defined EXTRA_SAFE
				if(colorTransfo.count(image->pixel(j)) == 0)
				{
					cerr<<"Error: trying to colorize image, pixel has no corresponding region"<<endl;
					exit (EXIT_FAILURE);
				}
			#endif
			image->pixel(j) = colorTransfo[image->pixel(j)];
		}
	}
}

FitsFile& writeTrackingRelations(FitsFile& file, const vector<Region*>& regions, const RegionGraph& tracking_graph, const Real pixel_area)
{
	vector<string> past_dates_obs;
	vector<ColorType> past_colors;
	vector<string> present_dates_obs;
	vector<ColorType> present_colors;
	vector<int> overlap_number_pixels;
	vector<Real> overlap_area_projected;
	
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		// For each region find all the edges comming in and extract the color and overlay size
		const RegionGraph::node* n = tracking_graph.get_node(regions[r]);
		for (RegionGraph::node::const_iterator it = n->in_begin(); it != n->in_end(); ++it)
		{
			past_dates_obs.push_back(it->from->get_region()->ObservationDate());
			past_colors.push_back(it->from->get_region()->Color());
			present_dates_obs.push_back(it->to->get_region()->ObservationDate());
			present_colors.push_back(it->to->get_region()->Color());
			overlap_number_pixels.push_back(it->weight);
			overlap_area_projected.push_back(it->weight * pixel_area);
		}
	}
	
	file.writeTable("TrackingRelations");
	file.writeColumn("PAST_DATE_OBS", past_dates_obs);
	file.writeColumn("PAST_COLOR", past_colors);
	file.writeColumn("PRESENT_DATE_OBS", present_dates_obs);
	file.writeColumn("PRESENT_COLOR", present_colors);
	file.writeColumn("OVERLAP_NUMBER_PIXELS", overlap_number_pixels);
	file.writeColumn("OVERLAP_AREA_PROJECTED", overlap_area_projected);
	
	return file;
}
