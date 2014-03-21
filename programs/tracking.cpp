//! This Program tracks regions between color maps.
/*!
@page tracking tracking.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/tracking.x [-option optionvalue ...]  [ fitsFile ... ] </tt>

@param fitsFile	Path of a fits files containing a maps of regions to track.

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param derotate	Set this to false if you dont want images to be derotated before comparison.

@param maxDeltaT	The maximal number of seconds between 2 tracked regions

@param newColor	The first color to attribute to a new untracked region

@param overlap	The number of images that overlap between 2 tracking run

@param recolorImages	Set this flag if you want all images to be colored and written to disk.Otherwise only the region table is updated.

@param regionTableName	The name of the region table Hdu

@param uncompressed	Set this flag if you want results maps to be uncompressed.

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/

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
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/Region.h"
#include "../classes/trackable.h"
#include "../classes/TrackingRelation.h"
#include "../classes/FitsFile.h"
#include "../classes/Header.h"


using namespace std;

string filenamePrefix;

int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// We declare our program description
	string programDescription = "This Program tracks regions between color maps.";
	programDescription+="\nVersion: 3.0";
	programDescription+="\nAuthor: Benjamin Mampaey, benjamin.mampaey@sidc.be";
	
	programDescription+="\nCompiled on "  __DATE__  " with options :";
	programDescription+="\nNUMBERCHANNELS: " + toString(NUMBERCHANNELS);
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\nVERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());
	
	// We define our program parameters
	ArgParser args(programDescription);
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["newColor"] = ArgParser::Parameter(0, 'n', "The first color to attribute to a new untracked region");
	args["maxDeltaT"] = ArgParser::Parameter(3600, 'd',"The maximal number of seconds between 2 tracked regions");
	args["overlap"] = ArgParser::Parameter(1, 'o',"The number of images that overlap between 2 tracking run");
	args["recolorImages"] = ArgParser::Parameter(false, 'A', "Set this flag if you want all images to be colored and written to disk.Otherwise only the region table is updated.");
	args["derotate"] = ArgParser::Parameter(true, 'D', "Set this to false if you dont want images to be derotated before comparison.");
	args["regionTableName"] = ArgParser::Parameter("Regions", 'H',"The name of the region table Hdu");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this flag if you want results maps to be uncompressed.");
	
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path of a fits files containing a maps of regions to track.");
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch ( const invalid_argument& error)
	{
		cerr<<"Error : "<<error.what()<<endl;
		cerr<<args.help_message(argv[0])<<endl;
		return EXIT_FAILURE;
	}
	
	newColor = args["newColor"];
	
	#ifdef HEK
		unsigned previous_last_hek_map = 0;
	#endif

	// We get the maps, regions and colors from the fits files
	vector<vector<Region*> > regions;
	vector<ColorMap*> images;
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned s = 0; s < imagesFilenames.size(); ++s)
	{
		FitsFile file(imagesFilenames[s]);
		// We get the image
		ColorMap* image = new ColorMap();
		image->readFits(file);
		
		// We crop the image
		image->nullifyAboveRadius(1);
		images.push_back(image);
		
		// We get te regions
		vector<Region* > tmp_regions;
		
		// If there is a table of regions, we use it to extract the regions
		if(file.has(args["regionTableName"]))
		{
			file.moveTo(args["regionTableName"].as<string>());
			readRegions(file, tmp_regions, true);
			Header tracking_info;
			file.readHeader(tracking_info);
			if(tracking_info.has("TNEWCOLR"))
			{
				ColorType latest_color = tracking_info.get<ColorType>("TNEWCOLR");
				newColor = latest_color > newColor ? latest_color : newColor;
			}
		}
		else // We extract the regions from the map
		{
			tmp_regions = getRegions(image);
			if(! (image->getHeader().has("TRACKED") && image->getHeader().get<bool>("TRACKED")))
			{
				for (unsigned r = 0; r < tmp_regions.size(); ++r)
					tmp_regions[r]->setColor(0);
			}
			if(image->getHeader().has("TNEWCOLR"))
			{
				ColorType latest_color = image->getHeader().get<ColorType>("TNEWCOLR");
				newColor = latest_color > newColor ? latest_color : newColor;
			}
		}

		#ifdef HEK
		// For the hek we need to know what was the last map that we wrote TrackingRelations in
		// (see at the end for an explanation)
		if (file.has("TrackingRelations"))
			previous_last_hek_map = s;
		#endif
		regions.push_back(tmp_regions);
	}
	
	filenamePrefix = images.size() > 0 ? toString(images[0]->ObservationTime()) + "." : "nofiles.";
	#if defined DEBUG
	// We output the regions found
	ouputRegions(regions, filenamePrefix+"regions_premodification.txt");
	#endif

	RegionGraph tracking_graph;
	// We create the nodes of the graph
	for (unsigned s = 0; s < regions.size(); ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			tracking_graph.add_node(regions[s][r]);
		}
	}


	//We ordonate the images according to time
	vector<unsigned> indices = imageOrder(images);
	// We create the edges of the graph
	// According to Cis we create an edge between 2 nodes
	// if their time difference is smaller than some value and
	// if they overlay and
	// if there is not already a path between them
	unsigned maxDeltaT = args["maxDeltaT"];
	for (unsigned d = 1; d < indices.size(); ++d)
	{	
		for (unsigned i = 0; d + i < indices.size(); ++i)
		{
			unsigned s1 = indices[i];
			unsigned s2 = indices[i + d];
			//If the time difference between the 2 images is too big, we don't need to continue
			unsigned delta_t = unsigned(difftime(images[s2]->ObservationTime(),images[s1]->ObservationTime()));
			if (delta_t > maxDeltaT)
			{
				continue;
			}	
			
			#if defined DEBUG
			if(args["derotate"])
			{
				SunImage<ColorType>* rotated = images[s1]->shifted_like(images[s2]);
				rotated->writeFits("rotated_"+ stripSuffix(stripPath(imagesFilenames[s1])) + "_to_" + stripSuffix(stripPath(imagesFilenames[s2]))+".fits");
				delete rotated;
			}
			#endif
			for (unsigned r1 = 0; r1 < regions[s1].size(); ++r1)
			{
				for (unsigned r2 = 0; r2 < regions[s2].size(); ++r2)
				{			
					if(!tracking_graph.get_node(regions[s1][r1])->path(tracking_graph.get_node(regions[s2][r2])))
					{		
						unsigned intersectPixels = 0;
						if(args["derotate"])
						{
							intersectPixels = overlay_derotate(images[s1], regions[s1][r1], images[s2], regions[s2][r2]);
						}
						else
						{
							intersectPixels = overlay(images[s1], regions[s1][r1], images[s2], regions[s2][r2]);
						}
						if(intersectPixels > 0)
						{
							tracking_graph.add_edge(tracking_graph.get_node(regions[s1][r1]), tracking_graph.get_node(regions[s2][r2]), intersectPixels);
						}
					}
				}

			}
		}
	}


	// To gain some memory space we can delete all images except if we need to recolor them
	if(!args["recolorImages"])
	{
		for (unsigned s = 0; s < images.size(); ++s)
			delete images[s];
	
	}

	#if defined DEBUG
	// We output the graph before tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_premodification", false);
	#endif

	//We color the graph
	const RegionGraph::iterator itnEnd = tracking_graph.end();
	for (RegionGraph::iterator itn = tracking_graph.begin(); itn != itnEnd; ++itn)
	{
		itn->colorize();
	}

	#if defined DEBUG
	// We output the graph after tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_postmodification");
	// We output the regions found
	ouputRegions(regions, filenamePrefix+"regions_postmodification.txt");
	#endif

	// We set whether we should not compress the maps
	const int compressed_fits = args["uncompressed"] ? 0 : FitsFile::compress;

	// We update the fits files with the new colors
	for (unsigned s = 0; s < images.size(); ++s)
	{
		FitsFile file(imagesFilenames[s], FitsFile::update);
		
		if(args["recolorImages"]) 
		{
			// We color the image and overwrite them in the fitsfile
			recolorFromRegions(images[s], regions[s]);
			images[s]->getHeader().set("TRACKED", true, "Map has been tracked");
			images[s]->writeFits(file, FitsFile::update|compressed_fits);
			delete images[s];
		}
		
		file.moveTo(args["regionTableName"].as<string>());
		
		// We write a nice header with info on the tracking
		Header tracking_info;
		tracking_info.set("TNEWCOLR", newColor, "Tracking latest color");
		tracking_info.set("TMAXDELT", maxDeltaT, "Tracking maxDeltaT");
		tracking_info.set("TOVERLAP", args["overlap"], "Tracking overlap");
		tracking_info.set("TDEROT", args["derotate"], "Tracking derotate");
		tracking_info.set("TNBRIMG", unsigned(imagesFilenames.size()), "Tracking number images");
		tracking_info.set("TRACKED", true, "Regions have been tracked");
		file.writeHeader(tracking_info);
		
		// We update the table of regions with the new colors and first_date_obs
		// We assume that the regions are in the same order than in the fits file(i.e. same id)
		vector<ColorType> tracked_colors(regions[s].size(),0);
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			tracked_colors[r] = regions[s][r]->Color();
		}
		file.writeColumn("TRACKED_COLOR", tracked_colors, FitsFile::overwrite);
		
		vector<string> first_observation_dates(regions[s].size());
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			first_observation_dates[r] = regions[s][r]->FirstObservationDate();
		}
		file.writeColumn("FIRST_DATE_OBS", first_observation_dates, FitsFile::overwrite);
		
		//If we recolor the images we update the color column
		if(args["recolorImages"]) 
			file.writeColumn("COLOR", tracked_colors, FitsFile::overwrite);
	}


	#ifndef HEK
	cout<<"Last color assigned: "<<newColor<<endl;	
	#else
	#error "This code has not been tested yet"
	
	/*	For The HEK we need to gives the relations between the hek events (<=> region in our program)
		The relations are of type follow, split and merge
		The relations can only concern events that have been witten to the HEK
		and should ignore any kind of split/merge that occured between regions no written to the HEK 
		This means that we need to find the relations between the AR of the last region map and the ones from the previous last_hek_map
		(i.e. the map used to write hek events after the previous call to tracking, it is the one that contains the table TrackingRelations) 
		
		map[previous_last_hek_map]
		map[previous_last_hek_map + 1]	|
		map[previous_last_hek_map + 2]	|
		map[previous_last_hek_map + 3]	| Must only contain colors from map[previous_last_hek_map]
		...				|
		map[last - 1]			|
		map[last]
	*/
	
	//We assume that when called by the hek, the regions are ordered by time
	
	// We recolor the graph top to bottom by giving to any child the color of its biggest parent
	unsigned last = regions.size() - 1;
	for (unsigned s = previous_last_hek_map + 1 ; s < last; ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			const RegionGraph::node* myBiggestParent = biggestParent( tracking_graph.get_node(regions[s][r]) );
			if(myBiggestParent)
			 	regions[s][r]->setColor( myBiggestParent->get_region()->Color() );
		}
	}
	
	#if defined DEBUG
	// We output the graph after recolorization
	// Ideally I should remove first all regions before previous_overlap from the graph, but there is no remove node in our graph libarry so I recolor them to black
	for (unsigned s = 0 ; s < previous_last_hek_map; ++s)
	{
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			 regions[s][r]->setColor(0);
		}
	}
	ouputGraph(tracking_graph, regions, "ar_graph_recolorization");
	#endif
	
	
	
	vector<TrackingRelation> relations;
	for (unsigned r = 0; r < regions[last].size(); ++r)
	{
		// We create for each region of map[last] the list of parents that have a color existing in map[previous_last_hek_map]
		const RegionGraph::node* n = tracking_graph.get_node(regions[last][r]);
		vector<ColorType> ancestors_color;
		for (RegionGraph::node::const_iterator it = n->in_begin(); it != n->in_end(); ++it)
		{
			for (unsigned r1 = 0; r1 < regions[previous_last_hek_map].size(); ++r1)
			{
				if(it->from->get_region()->Color() == regions[previous_last_hek_map][r1]->Color())
				{
					ancestors_color.push_back(regions[previous_last_hek_map][r1]->Color());
					break;
				}
			}
					

		}
		// Now that I know the list of my ancestors_color, I can create the relations
		if(ancestors_color.size() == 0)
		{
			// I have no ancestors, so I am a new color 
			relations.push_back(TrackingRelation(0,"new",regions[last][r]->Color()));
		}
		else if(ancestors_color.size() == 1 && ancestors_color[0] != regions[last][r]->Color())
		{
			// I have one ancestor of a different color, I am a split from him
			relations.push_back(TrackingRelation(ancestors_color[0],"splits_from",regions[last][r]->Color()));
		}
		else
		{
			// I have one or more ancestors
			// I am a follow if we have the same color
			// I am a merge of them if they have a different color
			for (unsigned a = 0; a < ancestors_color.size(); ++a)
			{
				if(ancestors_color[a] == regions[last][r]->Color())
				{
					relations.push_back(TrackingRelation(ancestors_color[a],"follows",regions[last][r]->Color()));
				}
				else
				{
					relations.push_back(TrackingRelation(ancestors_color[a],"merges_from",regions[last][r]->Color()));
				}
			}
		}
				
	}
	
	// We remove the duplicate relations
	sort(relations.begin(), relations.end());
	relations.erase(unique(relations.begin(), relations.end()), relations.end());
	
	// We write the relations in a table of the last file
	vector<ColorType> past_colors(relations.size());
	vector<ColorType> present_colors(relations.size());
	vector<string> relations_type(relations.size());
	for (unsigned r = 0; r < relations.size(); ++r)
	{
		past_colors[r] = relations[r].past_color;
		present_colors[r] = relations[r].present_color;
		relations_type[r] = relations[r].type;
		#if defined VERBOSE
		cout<<relations[r].past_color<<" "<<relations[r].type<<" "<<relations[r].present_color<<endl;
		#endif
	}
	FitsFile file(imagesFilenames[indices[indices.size()-1]], FitsFile::update);
	file.writeTable("TrackingRelations");
	file.writeColumn("PAST_COLOR", past_colors);
	file.writeColumn("RELATION_TYPE", relations_type);
	file.writeColumn("PRESENT_COLOR", present_colors);
	#endif
	
	return EXIT_SUCCESS;
}




