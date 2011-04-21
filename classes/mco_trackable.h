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

extern std::string filenamePrefix;
extern unsigned long newColor;

class TargetSet 
{
	public:
		std::vector<Region *> regions;
		double vx, vy;
};

class Vertex
{
	public:
		std::vector<StateVector> tracks;
};

typedef cgt::graph<Vertex *, int> InferenceGraph;
typedef cgt::graph<TargetSet *, int> TrackGraph;

#endif
