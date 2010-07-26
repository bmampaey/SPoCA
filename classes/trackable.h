#pragma once
#ifndef trackable_H
#define trackable_H


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>


#include "tools.h"
#include "constants.h"
#include "SunImage.h"
#include "Region.h"
#include "gradient.h"
#include "../cgt/graph.h"

extern std::string outputFileName;
extern unsigned long newColor;

typedef cgt::graph<Region*, int> RegionGraph;

//Ordonate the images according to time
void ordonate(std::vector<SunImage*>& images);

// Compute the number of pixels common to 2 regions from 2 images
unsigned overlay(SunImage* image1, const Region* region1, SunImage* image2, const Region* region2);

// Find the biggest parrent of a node (the one I have the biggest intersection with)
RegionGraph::node* biggestParent(const RegionGraph::node* n);

// Find the biggest son of a node (the one I have the biggest intersection with)
RegionGraph::node* biggestSon(const RegionGraph::node* n);

// Color a node
void colorize(RegionGraph::node& me);

// Tell if there is a path between a node and a region
bool path(const RegionGraph::node* n, const Region* r);

// Output a graph in the dot format
void ouputGraph(const RegionGraph& g, const std::vector<std::vector<Region*> >& regions, const std::string graphName);

// Output regions in the region format
void ouputRegions(const std::vector<std::vector<Region*> >& regions, std::string filename);


#endif
