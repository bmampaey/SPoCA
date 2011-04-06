//! Program that does tracking of regions on the sun 
/*!
@defgroup tracking tracking.x

 This program takes a colorized map in fits format as a mask of regions, and computes different statistics on the sun images provided
 
 
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
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/Region.h"
#include "../classes/trackable.h"
#include "../classes/TrackingRelation.h"
#include "../cgt/graph.h"
#include "../classes/FitsFile.h"
#include "../classes/Header.h"


using namespace std;
using namespace dsr;
using namespace cgt;

string outputFileName;

int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);

	// Options for the tracking
	unsigned newColorArg = 0;
	unsigned max_delta_t = 3600;
	unsigned overlap = 1;
	bool recolorImages = false;
	bool derotate = false;
	string regionTableHdu;
	
	#ifdef HEK
		unsigned previous_last_hek_map = 0;
	#endif
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Programm will track regions from color maps.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the maps of the regions to track.\n\t", imagesFilenames);
	arguments.new_named_unsigned_int('n',"newColor","positive integer","\n\tThe first color to attribute to a region\n\t",newColorArg);
	arguments.new_named_unsigned_int('d',"max_delta_t","positive integer","\n\tThe maximal delta time between 2 tracked regions\n\t",max_delta_t);
	arguments.new_named_unsigned_int('o',"overlap","positive integer","\n\tThe number of images that overlap between 2 tracking run\n\t",overlap);
	arguments.new_flag('A', "recolorImages", "\n\tSet this flag if you want all images to be colored and written to disk.\n\tOtherwise only the region table is updated.\n\t", recolorImages);
	arguments.new_flag('D', "derotate", "\n\tSet this flag if you want images to be derotated before comparison.\n\t", derotate);
	arguments.new_named_string('H',"regionTableHdu","string","\n\tThe name of the region table Hdu\n\t",regionTableHdu);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("2.0");
	arguments.process(argc, argv);

	newColor = newColorArg;

	// We get the maps, regions and colors from the fits files
	vector<vector<Region*> > regions;
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
		// We move to the table of regions
		file.moveTo(regionTableHdu);
		//We get the regions from the region table
		vector<Region* > tmp_regions;
		readRegions(file, tmp_regions);
		Header tracking_info;
		file.readHeader(tracking_info);
		if(tracking_info.get<bool>("TRACKED"))
		{
			//We get the tracked_colors from the table to update the regions color
			vector<ColorType> tracked_colors;
			file.readColumn("TRACKED_COLOR", tracked_colors);
			if(tracked_colors.size() == tmp_regions.size())
			{
				//We assume that the regions and the tracked_colors are in the same order (i.e. same id)
				for (unsigned r = 0; r < tmp_regions.size(); ++r)
				{
					tmp_regions[r]->setColor(tracked_colors[r]);
					// If the color is bigger than the newColor we increase newColor
					if(newColor <= tracked_colors[r])
						newColor = tracked_colors[r] + 1;
				}
			}
			if(tracking_info.get<bool>("TNEWCOLR"))
			{
				ColorType latest_color = tracking_info.get<ColorType>("TNEWCOLR");
				newColor = latest_color > newColor ? latest_color : newColor;
			}
		}
		else //We unset the colors of the region if the map had not been tracked yet
		{
			for (unsigned r = 0; r < tmp_regions.size(); ++r)
				tmp_regions[r]->setColor(0);
		}
		
		#ifdef HEK
		// For the hek we need to know what was the last map that we wrote TrackingRelations in
		// (see at the end for an explanation)
		if (file.has("TrackingRelations"))
			previous_last_hek_map = s;
		#endif
		regions.push_back(tmp_regions);
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


	//We ordonate the images according to time
	vector<unsigned> indices = imageOrder(images);
	// We create the edges of the graph
	// According to Cis we create an edge between 2 nodes
	// if their time difference is smaller than some value and
	// if they overlay and
	// if there is not already a path between them

	for (unsigned d = 1; d < indices.size(); ++d)
	{	
		for (unsigned i = 0; d + i < indices.size(); ++i)
		{
			unsigned s1 = indices[i];
			unsigned s2 = indices[i + d];
			//If the time difference between the 2 images is too big, we don't need to continue
			unsigned delta_t = unsigned(difftime(images[s2]->ObservationTime(),images[s1]->ObservationTime()));
			if (delta_t > max_delta_t)
			{
				continue;
			}	
			
			#if DEBUG >= 2
			if(derotate)
			{
				SunImage<ColorType>* rotated = images[s1]->rotated_like(images[s2]);
				rotated->writeFits("rotated_"+ stripSuffix(stripPath(imagesFilenames[s1])) + "_to_" + stripSuffix(stripPath(imagesFilenames[s2]))+".fits");
				delete rotated;
			}
			#endif
			for (unsigned r1 = 0; r1 < regions[s1].size(); ++r1)
			{
				for (unsigned r2 = 0; r2 < regions[s2].size(); ++r2)
				{			
					if(!path(tracking_graph.get_node(regions[s1][r1]), regions[s2][r2]))
					{		
						unsigned intersectPixels = 0;
						if(derotate)
						{
							intersectPixels = overlay_derotate(images[s1], regions[s1][r1], images[s2], regions[s2][r2]);
						}
						else
						{
							intersectPixels = overlay(images[s1], regions[s1][r1], images[s2], regions[s2][r2]);
						}
						if(intersectPixels > 0)
						{
							tracking_graph.insert_edge(intersectPixels, regions[s1][r1], regions[s2][r2]);
						}
					}
				}

			}
		}
	}


	// To gain some memory space we can delete all images except if we need to recolor them
	if(!recolorImages)
	{
		for (unsigned s = 0; s < images.size(); ++s)
			delete images[s];
	
	}

	#if DEBUG >= 2
	// We output the graph before tranformation
	ouputGraph(tracking_graph, regions, "ar_graph_premodification", false);
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

	// We update the fits files with the new colors
	for (unsigned s = 0; s < images.size(); ++s)
	{
		FitsFile file(imagesFilenames[s], FitsFile::update);
		
		if(recolorImages) 
		{
			// We color the image and overwrite them in the fitsfile
			recolorFromRegions(images[s], regions[s]);
			images[s]->header.set<string>("TRACKED", "yes", "Map has been tracked");
			images[s]->writeFits(file, FitsFile::update|FitsFile::compress);
			delete images[s];
		}
		
		file.moveTo(regionTableHdu);
		
		// We write a nice header with info on the tracking
		Header tracking_info;
		tracking_info.set<ColorType>("TNEWCOLR", newColor, "Tracking latest color");
		tracking_info.set<unsigned>("TMAXDELT", max_delta_t, "Tracking max_delta_t");
		tracking_info.set<unsigned>("TOVERLAP", overlap, "Tracking overlap");
		tracking_info.set<unsigned>("TDEROT", derotate?1:0, "Tracking derotate");
		tracking_info.set<unsigned>("TNBRIMG", imagesFilenames.size(), "Tracking number images");
		tracking_info.set<string>("TRACKED", "yes", "Regions have been tracked");
		file.writeHeader(tracking_info);
		
		// We update the table of regions with the new colors
		// We assume that the regions are in the same order than in the fits file(i.e. same id)
		vector<ColorType> tracked_colors(regions[s].size(),0);
		for (unsigned r = 0; r < regions[s].size(); ++r)
		{
			tracked_colors[r] = regions[s][r]->Color();
		}
		file.writeColumn("TRACKED_COLOR", tracked_colors, FitsFile::overwrite);
		
		//If we recolor the images we update the color column
		if(recolorImages) 
			file.writeColumn("COLOR", tracked_colors, FitsFile::overwrite);
	}


	#ifndef HEK
	cout<<"Last color assigned: "<<newColor<<endl;	
	#else
	
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
			 	regions[s][r]->setColor( myBiggestParent->value()->Color() );
		}
	}
	
	#if DEBUG >= 2
	// We output the graph after recolorization
	// Ideally I should remove first all regions before previous_overlap from the graph, but there is remove node in our graph libarry so I recolor them to black
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
		const RegionGraph::adjlist &parentsList = n->iadjlist();
		const RegionGraph::adjlist::const_iterator itadjEnd = parentsList.end();
		vector<ColorType> ancestors_color;
		for (RegionGraph::adjlist::const_iterator itadj = parentsList.begin(); itadj != itadjEnd; ++itadj)
		{
			for (unsigned r1 = 0; r1 < regions[previous_last_hek_map].size(); ++r1)
			{
				if(itadj->node().value()->Color() == regions[previous_last_hek_map][r1]->Color())
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
		#if DEBUG >= 3
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



