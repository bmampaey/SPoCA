#pragma once
#ifndef CoronalHole_H
#define CoronalHole_H

#include <vector>

#include "FeatureVector.h"
#include "ColorMap.h"
#include "Coordinate.h"

extern std::string outputFileName;

ColorMap* CoronalHoleMap(ColorMap* segmentedMap, unsigned CHclass);
unsigned CHclass(const std::vector<RealFeature>& B);
void blobsIntoCH (ColorMap* ARmap);

#endif

