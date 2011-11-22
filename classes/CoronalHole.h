#pragma once
#ifndef CoronalHole_H
#define CoronalHole_H

#include <vector>
#include <string>
#include "FeatureVector.h"
#include "EUVImage.h"
#include "ColorMap.h"
#include "Region.h"
#include "RegionStats.h"
#include "FitsFile.h"

/*!
@file CoronalHole.h
List of function to transform the results of the segmentation into a map of Coronal Hole (CH)
*/

//! Function that returns the class number corresponding to the CH
unsigned CHclass(const std::vector<RealFeature>& B);

//! Return a map of aggregated CH 
ColorMap* getAggregatedCHMap(const ColorMap* CHMap, const int projection = SunImage<ColorType>::no_projection);

//! Return a header with all the information about creating CH map
Header getCHMapHeader();

//! Method to write a CH map to a fits file
void writeCHMap(ColorMap*& CHMap, const std::string& filename, bool compressed = true, unsigned chaincodeMinPoints = 3, unsigned chaincodeMaxPoints = 0, Real chaincodeMaxDeviation = 0., EUVImage* image = NULL);
#endif

