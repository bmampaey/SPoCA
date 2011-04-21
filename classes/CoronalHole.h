#pragma once
#ifndef CoronalHole_H
#define CoronalHole_H

#include <vector>
#include "FeatureVector.h"
#include "ColorMap.h"

/*!
@file CoronalHole.h
List of function to transform the results of the segmentation into a map of Coronal Hole (CH)
*/

//! Function that creates the map of CH
ColorMap* CoronalHoleMap(const ColorMap* segmentedMap, unsigned CHclass, bool tresholdRawArea = false);
//! Function that returns the class number corresponding to the CH
unsigned CHclass(const std::vector<RealFeature>& B);
//! Routine that aggregate blobs into CH
void blobsIntoCH (ColorMap* CHmap);

#endif

