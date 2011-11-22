#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>
#include <string>
#include "FeatureVector.h"
#include "EUVImage.h"
#include "ColorMap.h"
#include "Region.h"
#include "RegionStats.h"
#include "FitsFile.h"

/*!
@file ActiveRegion.h
List of function to transform the results of the segmentation into a map of Active Region (AR)
*/

//! Function that returns the class number corresponding to the AR
unsigned ARclass(const std::vector<RealFeature>& B);

//! Return a map of aggregated AR 
ColorMap* getAggregatedARMap(const ColorMap* ARMap, const int projection = SunImage<ColorType>::no_projection);

//! Return a header with all the information about creating AR map
Header getARMapHeader();

//! Method to write a AR map to a fits file
void writeARMap(ColorMap*& ARMap, const std::string& filename, bool compressed = true, unsigned chaincodeMinPoints = 3, unsigned chaincodeMaxPoints = 0, Real chaincodeMaxDeviation = 0., EUVImage* image = NULL);

#endif

