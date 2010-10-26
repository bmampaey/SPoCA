#include "trackable.h"

using namespace std;
using namespace cgt;


unsigned long newColor;

// Compute the number of pixels common to 2 regions from 2 images, with derotation
unsigned overlay_derotate(SunImage* image1, const Region* region1, SunImage* image2, const Region* region2)
{
	unsigned intersectPixels = 0;
	PixelType setValue1 = image1->pixel(region1->FirstPixel());
	PixelType setValue2 = image2->pixel(region2->FirstPixel());
	
	
	// We are going to project the image1 into the coordinate of image2	
	Coordinate r1_boxmin = image1->shift_like(region1->Boxmin(), image2);
	Coordinate r1_boxmax = image1->shift_like(region1->Boxmax(), image2);
	Coordinate r2_boxmin = region2->Boxmin();
	Coordinate r2_boxmax = region2->Boxmax();

	//The projection of the box of region1 may lie outside of the sundisc ==> the projection is null 

	if(r1_boxmin == Coordinate(0))
		 r1_boxmin = r2_boxmin;
	if(r1_boxmax == Coordinate(0))
		 r1_boxmax = r2_boxmax;		


	// We compute the bornes of the intersection of teh 2 regions
	// If the 2 regions don't overlay, we will not even enter the loops
	unsigned Xmin = r1_boxmin.x > r2_boxmin.x ? r1_boxmin.x : r2_boxmin.x;
	unsigned Ymin = r1_boxmin.y > r2_boxmin.y ? r1_boxmin.y : r2_boxmin.y;
	unsigned Xmax = r1_boxmax.x < r2_boxmax.x ? r1_boxmax.x : r2_boxmax.x;
	unsigned Ymax = r1_boxmax.y < r2_boxmax.y ? r1_boxmax.y : r2_boxmax.y;

	// We scan the intersection in the coordinates of image2
	Coordinate c1, c2;
	for (c2.y = Ymin; c2.y <= Ymax; ++c2.y)
	{
		for (c2.x = Xmin; c2.x <= Xmax; ++c2.x)
		{
			// We project back the coordinate of image2 into the coordinate of image1
			c1 = image2->shift_like(c2, image1);
			// There is overlay between the two regions									  
			if(image1->pixel(c1) == setValue1 && image2->pixel(c2) == setValue2)
				++intersectPixels;
		}
	}
	
	
	return intersectPixels;

}


// Compute the number of pixels common to 2 regions from 2 images
unsigned overlay(SunImage* image1, const Region* region1, SunImage* image2, const Region* region2)
{
	unsigned intersectPixels = 0;
	PixelType setValue1 = image1->pixel(region1->FirstPixel());
	PixelType setValue2 = image2->pixel(region2->FirstPixel());
	
	Coordinate r1_boxmin = region1->Boxmin();
	Coordinate r1_boxmax = region1->Boxmax();
	Coordinate r2_boxmin = region2->Boxmin();
	Coordinate r2_boxmax = region2->Boxmax();

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
												  //There is overlay between the two regions
			if(image1->pixel(x,y) == setValue1 && image2->pixel(x,y) == setValue2)
				++intersectPixels;
		}
	}
	
	return intersectPixels;

}


// Find the biggest parrent of a node (the one I have the biggest intersection with)
RegionGraph::node* biggestParent(const RegionGraph::node* n)
{
	const RegionGraph::adjlist &parentsList = n->iadjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = parentsList.end();
	RegionGraph::adjlist::const_iterator biggest = parentsList.begin();
	if(biggest == parentsList.end())
		return NULL;
	for (RegionGraph::adjlist::const_iterator itadj = parentsList.begin(); itadj != itadjEnd; ++itadj)
	{
		if(itadj->edge().value() > biggest->edge().value())
			biggest = itadj;

	}
	return &(biggest->node());
}


// Find the biggest son of a node (the one I have the biggest intersection with)
RegionGraph::node* biggestSon(const RegionGraph::node* n)
{
	const RegionGraph::adjlist &sonsList = n->adjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = sonsList.end();
	RegionGraph::adjlist::const_iterator biggest = sonsList.begin();
	if(biggest == sonsList.end())
		return NULL;
	for (RegionGraph::adjlist::const_iterator itadj = sonsList.begin(); itadj != itadjEnd; ++itadj)
	{
		if(itadj->edge().value() > biggest->edge().value())
			biggest = itadj;

	}
	return &(biggest->node());
}



// Color a node
void colorize(RegionGraph::node& me)
{
	//If I'm already colored than I am fine
	if(me.value()->Color() != 0)
		return;

	// I need all my parents to have their color
	const RegionGraph::adjlist &parentsList = me.iadjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = parentsList.end();
	RegionGraph::adjlist::const_iterator biggestParent = parentsList.begin();
	for (RegionGraph::adjlist::const_iterator itadj = parentsList.begin(); itadj != itadjEnd; ++itadj)
	{
		colorize(itadj->node());				  //Carefull there is recursion here
		// We search for the biggest parent
		if(itadj->edge().value() > biggestParent->edge().value())
			biggestParent = itadj;

	}
	//Either I am the only child of my biggest parrent, or I am his biggest Son
	
	if(biggestParent != itadjEnd && me.value() == biggestSon(&(biggestParent->node()))->value() )
		me.value()->setColor(biggestParent->node().value()->Color());
	else					 //There was a split or a merge, or I have no parrents
		me.value()->setColor(++newColor);

}


// Tell if there is a path between a node and a region
bool path(const RegionGraph::node* n, const Region* r)
{
	if (n->value() == r)
		return true;

	const RegionGraph::adjlist &adjList = n->adjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = adjList.end();
	for (RegionGraph::adjlist::const_iterator itadj = adjList.begin(); itadj != itadjEnd; ++itadj)
	{
		if(path(&(itadj->node()), r))
			return true;

	}
	return false;
}


// Output a graph in the dot format
void ouputGraph(const RegionGraph& g, const vector<vector<Region*> >& regions, const string graphName)
{

	ofstream graphFile((outputFileName + graphName + ".dot").c_str());
	if (graphFile.good())
	{

		graphFile<<"digraph "<<graphName<<" {"<<endl;
		graphFile<<"node [colorscheme=set312 penwidth=4];"<<endl;
		//First we output all node info
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			string rank;
			for (unsigned r = 0; r < regions[s].size(); ++r)
			{
				rank += " \"" + regions[s][r]->HekLabel() + "\"";
				graphFile <<"\""<< regions[s][r]->HekLabel()<<"\"";
				if(regions[s][r]->Color() != 0)
					graphFile<<dot_gradient[ (int(regions[s][r]->Color()) % gradientMax) + 1 ] << endl;
				else
					graphFile<<" [color="<< (s%12) + 1 <<"];" << endl;

			}
			graphFile<<"{ rank=same; "<< rank <<" };" << endl;

		}
		// Then we output all edges info
		const RegionGraph::const_iterator itnEnd = g.end();
		for (RegionGraph::const_iterator itn = g.begin(); itn != itnEnd; ++itn)
		{
			const RegionGraph::adjlist &adjList = itn->adjlist();
			const RegionGraph::adjlist::const_iterator itadjEnd = adjList.end();
			for (RegionGraph::adjlist::const_iterator itadj = adjList.begin(); itadj != itadjEnd; ++itadj)
			{
				graphFile <<"\""<< itn->value()->HekLabel()<<"\" -> \"" << itadj->node().value()->HekLabel()<< "\" [label="<<itadj->edge().value()<<"];" << endl;
			}

		}
		graphFile<<"}"<<endl;
	}
	graphFile.close();
}


// Output regions in the region format
void ouputRegions(const vector<vector<Region*> >& regions, string filename)
{
	ofstream regionFile(filename.c_str());
	if (regionFile.good())
	{
		regionFile<<Region::header<<endl<<endl;
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			for (unsigned r = 0; r < regions[s].size(); ++r)
			{
				regionFile<<regions[s][r]->toString()<<endl;
			}
			regionFile<<endl;
		}
	}
	regionFile.close();
}

// Comparison of 2 images according to time (for sorting)
inline bool compare(const SunImage* a, const SunImage* b)
{
	return a->ObservationTime() < b->ObservationTime();
}

//Ordonate the images according to time
void ordonate(vector<SunImage*>& images)
{
	sort(images.begin(), images.end(), compare);

	#if DEBUG >= 1
	//We remove the ones that have duplicate time
	vector<SunImage*>::iterator s1 = images.begin();
	vector<SunImage*>::iterator s2 = images.begin() + 1;
	while (s2 != images.end())
	{
		if (unsigned(difftime((*s2)->ObservationTime(),(*s1)->ObservationTime())) == 0)
		{
			s1 = images.erase(s1,s2);
		}
		else
		{
			++s1;
		}
		s2 = s1 + 1;
	}
	#endif
}

void recolorFromRegions(SunImage* image, const vector<Region*>& regions)
{
	vector<unsigned> colorTransfo(regions.size() + 1);
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		unsigned pixelcolor = unsigned(image->pixel(regions[r]->FirstPixel()));
		if(pixelcolor >= colorTransfo.size())
			colorTransfo.resize(pixelcolor + 100);
		colorTransfo[pixelcolor] = regions[r]->Color();

	}
	for (unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		if(image->pixel(j) != image->nullvalue())
		{
			#if DEBUG >= 1
				if(unsigned(image->pixel(j)) > colorTransfo.size())
				{
					cerr<<"ERROR trying to colorize image, pixel has no corresponding region"<<endl;
					exit (EXIT_FAILURE); 
				}
			#endif
			image->pixel(j) = colorTransfo[unsigned(image->pixel(j))];
		}
	}
}
