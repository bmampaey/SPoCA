// This programm will do tracking of regions from color maps
// Written by Benjamin Mampaey on 15 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/SunImage.h"
#include "../classes/Region.h"
#include "../classes/gradient.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/MainUtilities.h"
#include "../classes/CoordinateConvertor.h"
#include "../cgt/graph.h"


using namespace std;
using namespace dsr;
using namespace cgt;

string outputFileName;
unsigned long newColor;

typedef graph<Region*, int> RegionGraph;

// Comparison of 2 images according to time (for sorting)
inline bool compare(const SunImage* a, const SunImage* b)
{
	return a->ObservationTime() < b->ObservationTime();
}


// Compute the number of pixels common to 2 regions from 2 images
unsigned overlay(SunImage* image1, const Region* region1, SunImage* image2, const Region* region2)
{
	unsigned intersectPixels = 0;
	PixelType setValue1 = image1->pixel(region1->FirstPixel());
	PixelType setValue2 = image2->pixel(region2->FirstPixel());
	unsigned Xmin = region1->Boxmin().x > region2->Boxmin().x ? region1->Boxmin().x : region2->Boxmin().x;
	unsigned Ymin = region1->Boxmin().y > region2->Boxmin().y ? region1->Boxmin().y : region2->Boxmin().y;
	unsigned Xmax = region1->Boxmax().x < region2->Boxmax().x ? region1->Boxmax().x : region2->Boxmax().x;
	unsigned Ymax = region1->Boxmax().y < region2->Boxmax().y ? region1->Boxmax().y : region2->Boxmax().y;

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
RegionGraph::node& biggestParent(RegionGraph::node& n)
{
	const RegionGraph::adjlist &parentsList = n.iadjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = parentsList.end();
	RegionGraph::adjlist::const_iterator biggest = parentsList.begin();
	for (RegionGraph::adjlist::const_iterator itadj = parentsList.begin(); itadj != itadjEnd; ++itadj)
	{
		if(itadj->edge().value() > biggest->edge().value())
			biggest = itadj;

	}
	return biggest->node();
}


// Find the biggest son of a node (the one I have the biggest intersection with)
RegionGraph::node& biggestSon(RegionGraph::node& n)
{
	const RegionGraph::adjlist &sonsList = n.adjlist();
	const RegionGraph::adjlist::const_iterator itadjEnd = sonsList.end();
	RegionGraph::adjlist::const_iterator biggest = sonsList.begin();
	for (RegionGraph::adjlist::const_iterator itadj = sonsList.begin(); itadj != itadjEnd; ++itadj)
	{
		if(itadj->edge().value() > biggest->edge().value())
			biggest = itadj;

	}
	return biggest->node();
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
	if(biggestParent != itadjEnd && me.value() == biggestSon(biggestParent->node()).value())
		me.value()->setColor(biggestParent->node().value()->Color());
	else										  //There was a split or a merge, or I have no parrents
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
	string filename = outputFileName + graphName + ".dot";
	ofstream graphFile(filename.c_str());
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
				rank += " \"" + regions[s][r]->Label() + "\"";
				graphFile <<"\""<< regions[s][r]->Label()<<"\"";
				if(regions[s][r]->Color() != 0)
					graphFile<<" [color=\""<< gradient[ (int(regions[s][r]->Color()) % gradientMax) + 1 ] <<"\"];" << endl;
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
				graphFile <<"\""<< itn->value()->Label()<<"\" -> \"" << itadj->node().value()->Label()<< "\" [label="<<itadj->edge().value()<<"];" << endl;
			}

		}
		graphFile<<"}"<<endl;
	}
	graphFile.close();
}


// Output regions in the region format
void ouputRegions(const vector<vector<Region*> >& regions, string filename)
{
	filename = outputFileName + filename;
	ofstream regionFile(filename.c_str());
	if (regionFile.good())
	{
		regionFile<<Region::header<<endl<<endl;
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			for (unsigned r = 0; r < regions[s].size(); ++r)
			{
				regionFile<<*(regions[s][r])<<endl;
			}
			regionFile<<endl;
		}
	}
	regionFile.close();
}


int main(int argc, const char **argv)
{
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// Options for the tracking
	newColor = 0;
	unsigned delta_time = 3600;
	unsigned overlap = 1;
	
	// The list of names of the sun images to process
	string imageType = "AIA";
	vector<string> sunImagesFileNames;

	string programDescription = "This Programm will track regions from color maps.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the maps of the regions to track.\n\t", sunImagesFileNames);
	arguments.new_named_unsigned_long('n',"newColor","positive integer","\n\tThe last color given to active regions\n\t",newColor);
	arguments.new_named_unsigned_int('d',"delta_time","positive integer","\n\tThe maximal delta time between 2 tracked regions\n\t",delta_time);
	arguments.new_named_unsigned_int('D',"overlap","positive integer","\n\tThe number of images that overlap between 2 tracking run\n\t",overlap);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// General variables
	vector<SunImage*> images;

	// We read the color maps
	images = getImagesFromFiles(imageType, sunImagesFileNames, true);
	// We crop the images
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing("NAR", 1);
	}

	//We ordonate the images according to time
	sort(images.begin(), images.end(), compare);

	#if defined(DEBUG) && DEBUG >= 1
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
	
	// We get the regions out of the images
	vector<vector<Region*> > regions;
	for (unsigned s = 0; s < images.size(); ++s)
	{
		regions.push_back(getRegions(images[s]));
	}

	//We use the colors of the first image as a reference
	for (unsigned r = 0; r < regions[0].size(); ++r)
	{
		regions[0][r]->setColor((long unsigned)images[0]->pixel(regions[0][r]->FirstPixel()));
		// If the color is bigger than the newColor we increase newColor
		if(newColor < regions[0][r]->Color())
			newColor = regions[0][r]->Color();
	}
	
	#if defined(DEBUG) && DEBUG >= 2
	// We output the regions found
	ouputRegions(regions, "regions_premodification.txt");
	#endif

	RegionGraph tracking_graph;
	// We create the nodes of the graph
	for (unsigned s = 0; s < regions.size(); ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			tracking_graph.insert_vertex(regions[s][r]);
		}

	}

	// We create the edges of the graph
	// According to Cis we create an edge between 2 nodes
	// if their time difference is smaller than some value and
	// if they overlay and
	// if there is not already a path between them

	
	for (unsigned d = 1; d < images.size(); ++d)
	{	
		for (unsigned s1 = 0; d + s1 < images.size(); ++s1)
		{
			unsigned s2 = s1 + d;
			//If the time difference between the 2 images is too big, we don't need to continue
			if (unsigned(difftime(images[s2]->ObservationTime(),images[s1]->ObservationTime())) > delta_time)
			{
				break;						  
			}	
			for (unsigned r1 = 0; r1 < regions[s1].size(); ++r1)
			{
				for (unsigned r2 = 0; r2 < regions[s2].size(); ++r2)
				{
					unsigned intersectPixels = overlay(images[s1], regions[s1][r1], images[s2], regions[s2][r2]);
					if(intersectPixels > 0 && !path(tracking_graph.get_node(regions[s1][r1]), regions[s2][r2]))
					{
						tracking_graph.insert_edge(intersectPixels, regions[s1][r1], regions[s2][r2]);
					}
				}

			}
		}
	}

	// To gain some memory space we can delete all images but the last ones
	#if !defined(DEBUG) || DEBUG < 2			  // We must keep them in case of debug because we display them
	for (unsigned s = 0; s < images.size() - overlap; ++s)
		delete images[s];
	#endif

	#if defined(DEBUG) && DEBUG >= 2
	// We output the graph before tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_premodification");
	#endif

	//We color the graph
	const RegionGraph::iterator itnEnd = tracking_graph.end();
	for (RegionGraph::iterator itn = tracking_graph.begin(); itn != itnEnd; ++itn)
	{
		colorize(*itn);
	}

	#if defined(DEBUG) && DEBUG >= 2
	// We output the graph after tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_postmodification");
	// We output the regions found
	ouputRegions(regions, "regions_postmodification.txt");
	#endif


	
	#if defined(DEBUG) && DEBUG >= 2
	// We color the first images and output them
	for (unsigned s = 0; s + overlap < images.size(); ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			if(images[s]->pixel(regions[s][r]->FirstPixel()) != regions[s][r]->Color())
				images[s]->propagateColor(regions[s][r]->Color(), regions[s][r]->FirstPixel());
		}
		images[s]->writeFitsImage(outputFileName + sunImagesFileNames[s].substr(0, sunImagesFileNames[s].rfind(".fits")) + ".tracked.fits");
		delete images[s];
	}
	#endif
	//We color the last images and output them
	for (unsigned s = images.size() - overlap; s < images.size(); ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			if(images[s]->pixel(regions[s][r]->FirstPixel()) != regions[s][r]->Color())
				images[s]->propagateColor(regions[s][r]->Color(), regions[s][r]->FirstPixel());
		}
		images[s]->writeFitsImage(outputFileName + sunImagesFileNames[s].substr(0, sunImagesFileNames[s].rfind(".fits")) + ".tracked.fits");
		delete images[s];

	}

	
	cout<<"Last color assigned: "<<newColor<<endl;
	
	/* For Tufjan Kolak 3D visualisation program
	//We output the regions
	for (unsigned s = 0; s + overlap < images.size(); ++s)
	{
		cout<<"IMAGE "<<sunImagesFileNames[s]<<endl;
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			cout<<*(regions[s][r])<<endl;
		}
		cout<<"IMAGEEND"<<endl;
	}
	*/
	

	//We output the regions
	for (unsigned s = 0; s  < images.size(); ++s)
	{
		cout<<"IMAGE "<<images[s]->ObservationDate()<<endl;
		CoordinateConvertor cc(images[s]);
		float x = 0, y = 0;
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			cc.convert("HGC", regions[s][r]->Center(), x, y);
			cout<<x<<" "<<y<<" "<<regions[s][r]->Color()<<endl;
		}
		cout<<"IMAGEEND"<<endl;
	}

	

	return EXIT_SUCCESS;
}
