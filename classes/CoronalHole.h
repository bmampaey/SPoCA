#pragma once
#ifndef CoronalHole_H
#define CoronalHole_H

#include <vector>

#include "FeatureVector.h"
#include "ColorMap.h"
#include "EUVImage.h"
#include "Coordinate.h"
#include "CoronalHoleStats.h"

extern std::string outputFileName;

ColorMap* CoronalHoleMap(const ColorMap* segmentedMap, unsigned CHclass, bool tresholdRawArea = false);
unsigned CHclass(const std::vector<RealFeature>& B);
void blobsIntoCH (ColorMap* ARmap);

#endif

