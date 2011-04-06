#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>

#include "FeatureVector.h"
#include "ColorMap.h"
#include "EUVImage.h"
#include "Coordinate.h"
#include "ActiveRegionStats.h"

extern std::string outputFileName;


ColorMap* ActiveRegionMap(const ColorMap* segmentedMap, unsigned ARclass, bool tresholdRawArea = false);
unsigned ARclass(const std::vector<RealFeature>& B);
void blobsIntoAR (ColorMap* ARmap);

#endif

