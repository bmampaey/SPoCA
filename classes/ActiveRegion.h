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

//! Return a map of aggregated AR 
ColorMap* getAggregatedARMap(const ColorMap* ARMap, const int projection = SunImage<ColorType>::no_projection);

//! Fill a header with all the information about AR map creation
void fillHeaderAR(Header& header);

//! Method to write a AR map to a fits file
void writeARMap(ColorMap*& ARMap, const std::string& filename, std::vector<EUVImage*> images, bool compressed = true, unsigned chaincodeMinPoints = 3, unsigned chaincodeMaxPoints = 0, Real chaincodeMaxDeviation = 0.);

#endif

