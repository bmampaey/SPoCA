#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>

#include "FeatureVector.h"
#include "SunImage.h"

SunImage* ActiveRegionMap(SunImage* segmentedMap, unsigned ARclass);
SunImage* CoronalHoleMap(SunImage* segmentedMap, unsigned CHclass);

unsigned ARclass(const std::vector<RealFeature>& B);
unsigned CHclass(const std::vector<RealFeature>& B);
void blobsIntoAR (SunImage* ARmap);

#endif

