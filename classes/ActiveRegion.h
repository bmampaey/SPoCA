#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>
#include "FeatureVector.h"
#include "ColorMap.h"

/*!
@file ActiveRegion.h
List of function to transform the results of the segmentation into a map of Active %Region (AR)
*/

//! Function that creates the map of AR
ColorMap* ActiveRegionMap(const ColorMap* segmentedMap, unsigned ARclass, bool thresholdRawArea = false);
//! Function that returns the class number corresponding to the AR
unsigned ARclass(const std::vector<RealFeature>& B);
//! Routine that aggregate blobs into AR
void blobsIntoAR (ColorMap* ARmap);

#endif

