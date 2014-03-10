//! Program that does tracking of regions on the sun 
/*!
@page tracking tracking.x

 This program takes several colorized map of sun regions, and will recolor them so that their color remains constant trough time.
 
 The method for the tracking has been described by Cis Verbeeck in "Tracking Active Regions detected by SPoCA"
 
 @section usage Usage
 
 <tt> tracking.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> tracking.x [-option optionvalue, ...] colorizeMap1 colorizeMap2 </tt>
 
 You must provide at least one colorized map.
 
@param newColor	The first color to give to a region

@param max_delta_t	The maximal delta time between 2 tracked regions

@param overlap	The number of images that overlap between 2 tracking run

@param recolorImages	Set this flag if you want all images to be colored and written to disk.
Otherwise only the region table is updated.

@param derotate	Set this flag if you want images to be derotated before comparison.

@param regionTableHdu	The name of the region table Hdu

@param uncompressed_results	Set this flag if you want results maps to be uncompressed.

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>

#include "../classes/trackable.h"
#include "../classes/constants.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/TrackingRegion.h"
#include "../classes/TrackingEdge.h"


using namespace std;
using namespace dsr;

string filenamePrefix;


int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);

	// Program version
	string version = "2.0";

	// Options for the tracking
	unsigned max_delta_t = 0;
	bool derotate = false;
	bool unique;
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;
	
	// Option for the output file
	string output = "graph.json";
	
	string programDescription = "This Programm will extract the graph of regions from color maps.\n";
	programDescription+="Compiled with options :";
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\VERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the maps of the regions to track.\n\t", imagesFilenames);
	arguments.new_named_string('O', "output","output file name", "\n\tThe name for the output file.\n\t", output);
	arguments.new_named_unsigned_int('d',"max_delta_t","positive integer","\n\tThe maximal number of seconds between 2 tracked regions\n\t",max_delta_t);
	arguments.new_flag('U', "unique", "\n\tSet this flag if you want at most one path between 2 nodes.\n\t", unique);
	arguments.new_flag('D', "derotate", "\n\tSet this flag if you want images to be derotated before comparison.\n\t", derotate);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version.c_str());
	arguments.process(argc, argv);
	
	// We get the maps, regions and colors from the fits files
	vector<vector<TrackingRegion*> > regions;
	vector<ColorMap*> images;
	for (unsigned s = 0; s < imagesFilenames.size(); ++s)
	{
		FitsFile file(imagesFilenames[s]);
		// We get the image
		ColorMap* image = new ColorMap();
		image->readFits(file);
		
		// We crop the image
		image->nullifyAboveRadius(1);
		images.push_back(image);
		
		// We extract the regions from the map
		regions.push_back(getTrackingRegions(image));
	}
	
	//We ordonate the images according to time
	vector<unsigned> indices = imageOrder(images);
	
	// We extract the edges of the graph
	vector< vector<TrackingEdge*> > edges;
	if(max_delta_t == 0)
	{
		// We only extract the edges between 2 consecutive images
		for (unsigned i = 0; i < indices.size() - 1; ++i)
		{
			unsigned s1 = indices[i];
			unsigned s2 = indices[i + 1];
			edges.push_back(get_edges(images[s1], regions[s1], images[s2], regions[s2], derotate));
		}
	}
	else
	{
		// We extract the edges to all following images
		for (unsigned d = 1; d < indices.size(); ++d)
		{	
			for (unsigned i = 0; d + i < indices.size(); ++i)
			{
				unsigned s1 = indices[i];
				unsigned s2 = indices[i + d];
				//If the time difference between the 2 images is too big, we don't need to continue
				unsigned delta_t = unsigned(difftime(images[s2]->ObservationTime(),images[s1]->ObservationTime()));
				if (delta_t <= max_delta_t)
				{
					edges.push_back(get_edges(images[s1], regions[s1], images[s2], regions[s2], derotate));
				}
			}
		}
	}
	
	// We write the graph JSON to file
	ofstream output_file(output.c_str());
	output_file<<"{\n";
	// We write the nodes
	output_file<<"\"nodes\" : [\n";
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		for (unsigned rr = 0; rr < regions[r].size(); ++rr)
		{
			output_file<<regions[r][rr]->toJSON()<<",\n";
		}
	}
	output_file.seekp(-2, ios_base::cur);
	output_file<<"],"<<endl;
	
	// We write the edges
	output_file<<"\"edges\" : [\n";
	for (unsigned e = 0; e < edges.size(); ++e)
	{
		for (unsigned ee = 0; ee < edges[e].size(); ++ee)
		{
			output_file<<edges[e][ee]->toJSON()<<",\n";
		}
	}
	output_file.seekp(-2, ios_base::cur);
	output_file<<"]"<<endl;
	
	output_file<<"}\n";
	output_file.close();
	return EXIT_SUCCESS;
}




