// This programm will do tracking of regions from color maps
// Written by Benjamin Mampaey on 15 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/SunImage.h"
#include "../classes/Region.h"
#include "../classes/trackable.h"
#include "../cgt/graph.h"


using namespace std;
using namespace dsr;
using namespace cgt;

string outputFileName;


int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);

	// Options for the tracking
	newColor = 0;
	unsigned delta_time = 3600;
	unsigned overlap = 1;
	bool writeAllImages = false;
	
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
	arguments.new_flag('A', "writeAllImages", "\n\tSet this flag if you want all images to be colored and written to disk.\n\tOtherwise only the image for the next tracking will be colored and written.\n\t", writeAllImages);
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We read the color maps
	vector<SunImage*> images = getImagesFromFiles(imageType, sunImagesFileNames, true);

	//We ordonate the images according to time
	ordonate(images);

	// We crop the images
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing("NAR", 1);
	}


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
	
	#if DEBUG >= 2
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

	// To gain some memory space we can delete all images but the one used for the next tracking
	unsigned firstImageNextTracking = images.size() - overlap > 0 ? images.size() - overlap : 0;
	if(!writeAllImages)
	{
		for (unsigned s = 0; s < images.size(); ++s)
			if(s != firstImageNextTracking)
			{
				delete images[s];
				images[s] = NULL;
			}
	}

	#if DEBUG >= 2
	// We output the graph before tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_premodification");
	#endif

	//We color the graph
	const RegionGraph::iterator itnEnd = tracking_graph.end();
	for (RegionGraph::iterator itn = tracking_graph.begin(); itn != itnEnd; ++itn)
	{
		colorize(*itn);
	}

	#if DEBUG >= 2
	// We output the graph after tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_postmodification");
	// We output the regions found
	ouputRegions(regions, "regions_postmodification.txt");
	#endif

	
	
	if(writeAllImages) // We color all the images and output them
	{
		for (unsigned s = 0; s < images.size(); ++s)
		{
			recolorFromRegions(images[s], regions[s]);
			images[s]->writeFitsImage(outputFileName + sunImagesFileNames[s]);
			delete images[s];
		}
	}
	else //We color the image used for the next tracking
	{
		recolorFromRegions(images[firstImageNextTracking], regions[firstImageNextTracking]);
		images[firstImageNextTracking]->writeFitsImage(outputFileName + sunImagesFileNames[firstImageNextTracking]);
		delete images[firstImageNextTracking];
	}
	

	#ifndef HEK
	cout<<"Last color assigned: "<<newColor<<endl;	
	#else
	
	//We output the number of Active Events and the last color assigned
	cout<<regions[regions.size() - 1].size()<<" "<<newColor<<endl;

	// We output the relations between the AR of the last region map and the ones from the previous last (i.e. the last from the previous call to tracking) 
	
	/* First we recolor the graph top to bottom by giving to any child the color of its biggest parent
	map[previous_last] 	(== overlap - 1)
	map[previous_last + 1]	|
	map[previous_last + 2]	|
	map[previous_last + 3]	| Must only contain colors from map[previous_last]
	...					|
	map[last - 1]			|
	map[last]
	*/
	
	unsigned last = regions.size() - 1;
	unsigned previous_last = overlap - 1;
	
	for (unsigned s = previous_last + 1 ; s < last; ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			const RegionGraph::node* myBiggestParent = biggestParent( tracking_graph.get_node(regions[s][r]) );
			if(myBiggestParent)
			 	regions[s][r]->setColor( myBiggestParent->value()->Color() );
		}
	}
	
	#if DEBUG >= 2
	// We output the graph after recolorization
	// Ideally I should remove first all regions before previous_overlap from the graph 
	ouputGraph(tracking_graph, regions, "ar_graph_recolorization");
	#endif
	
	
	// Now we create for each region of map[last] the list of parents that have a color existing in map[previous_last]

	for (unsigned r = 0; r < regions[last].size(); ++r)
	{
		const RegionGraph::node* n = tracking_graph.get_node(regions[last][r]);
		const RegionGraph::adjlist &parentsList = n->iadjlist();
		const RegionGraph::adjlist::const_iterator itadjEnd = parentsList.end();
		vector<unsigned> ancestors_color;
		for (RegionGraph::adjlist::const_iterator itadj = parentsList.begin(); itadj != itadjEnd; ++itadj)
		{
			for (unsigned r1 = 0; r1 < regions[previous_last].size(); ++r1)
			{
				if(itadj->node().value()->Color() == regions[previous_last][r1]->Color())
				{
					ancestors_color.push_back(regions[previous_last][r1]->Color());
					break;
				}
			}
					

		}
		// Now that I know the list of my ancestors_color, I can output the relations
		if(ancestors_color.size() == 0)
		{
			// I have no ancestors, so I am a new color 
			cout<< regions[last][r]->Color() << " new " << 0 <<endl;
		}
		else if(ancestors_color.size() == 1 && ancestors_color[0] != regions[last][r]->Color())
		{
			// I have one ancestor of a different color, I am a split from him
			cout<< regions[last][r]->Color() << " splits_from " <<  ancestors_color[0]<<endl;
		}
		else
		{
			// I have many ancestors, I am a merge of them (unless we have the same color then I am a follow)
			for (unsigned a = 0; a < ancestors_color.size(); ++a)
			{
				if(ancestors_color[a] == regions[last][r]->Color())
				{
					cout<<  regions[last][r]->Color() << " follows " <<  ancestors_color[a] <<endl;
				}
				else
				{
					cout<<  regions[last][r]->Color() << " merges_from " << ancestors_color[a] <<endl;
				}
			}
		}
				
	}
	
	#endif
	
	return EXIT_SUCCESS;
}
