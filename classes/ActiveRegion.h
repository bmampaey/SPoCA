#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>

#include "FeatureVector.h"
#include "ColorMap.h"
#include "Coordinate.h"

extern std::string outputFileName;

ColorMap* ActiveRegionMap(ColorMap* segmentedMap, unsigned ARclass);
unsigned ARclass(const std::vector<RealFeature>& B);
void blobsIntoAR (ColorMap* ARmap);

#endif

