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

#include "../classes/ColorMap.h"
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
	bool writeAllRegions = false;
	
	// The list of names of the sun images to process
	vector<string> regionsFilenames;

	string programDescription = "This Programm will track regions from color maps.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("regionsFileName1 regionsFileName2 ...", "\n\tThe name of the files containing the regions to track.\n\t", regionsFilenames);
	arguments.new_named_unsigned_long('n',"newColor","positive integer","\n\tThe last color given to active regions\n\t",newColor);
	arguments.new_named_unsigned_int('d',"delta_time","positive integer","\n\tThe maximal delta time between 2 tracked regions\n\t",delta_time);
	arguments.new_named_unsigned_int('D',"overlap","positive integer","\n\tThe number of images that overlap between 2 tracking run\n\t",overlap);
	arguments.new_flag('A', "writeAllRegions", "\n\tSet this flag if you want all regions files to be rewritten to disk with the correct color.\n\tOtherwise only the files for the next tracking will be rewritten.\n\t", writeAllRegions);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We read the regions
	vector<vector<Region*> > regions = getRegionsFromFiles(regionsFilenames);


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
	// We create an edge between 2 nodes
	// if their time difference is smaller than some value and
	// if they distance is smaller than TRACKING_MAX_DISTANCE
	// if there is not already a path between them

	
	for (unsigned d = 1; d < regions.size(); ++d)
	{	
		for (unsigned s1 = 0; d + s1 < regions.size(); ++s1)
		{
			unsigned s2 = s1 + d;
			//If the time difference between the 2 regions sets is too big, we don't need to continue
			if (unsigned(difftime(regions[s2][0]->ObservationTime(),images[s1][0]->ObservationTime())) > delta_time)
			{
				break;						  
			}	
			for (unsigned r1 = 0; r1 < regions[s1].size(); ++r1)
			{
				for (unsigned r2 = 0; r2 < regions[s2].size(); ++r2)
				{
					Real distance = distance(regions[s1][r1], regions[s2][r2]);
					if(distance <= TRACKING_MAX_DISTANCE && !path(tracking_graph.get_node(regions[s1][r1]), regions[s2][r2]))
					{
						tracking_graph.insert_edge(distance, regions[s1][r1], regions[s2][r2]);
					}
				}

			}
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
	#endif

	
	
	if(writeAllRegions) // We write all the regions files
	{
		for (unsigned s = 0; s < regions.size(); ++s)
		{
			writeRegionsToFile(regions[s]);
		}
	}
	else //We write only the regions files for the next tracking
	{
		for (unsigned s = firstImageNextTracking; s < images.size(); ++s)
		{
			writeRegionsToFile(regions[s]);
		}
	}
	


	
	return EXIT_SUCCESS;
}
