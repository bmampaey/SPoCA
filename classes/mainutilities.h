// Functions to help main programs

#pragma once
#ifndef MainUtilities_H
#define MainUtilities_H

#include <vector>
#include <deque>
#include <iostream>
#include <string>
//#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sys/stat.h>
#include <limits>
#include <stdexcept>

#include "FeatureVector.h"
#include "EUVImage.h"
#include "EITImage.h"
#include "EUVIImage.h"
#include "AIAImage.h"
#include "SWAPImage.h"
#include "HMIImage.h"
#include "SUVIImage.h"
#include "ColorMap.h"
#include "Header.h"
#include "Coordinate.h"


extern std::string filenamePrefix;
extern const char * instruments[];
extern const char * limb_corrections[];

//! Read class centers and their associated channels from a file
bool readCentersFromFile(const std::string& centersFileName, std::vector<std::string>& channels, std::vector<RealFeature>& B);

//! Read a list of class centers and their associated channels from a file
bool readCentersFromFile(const std::string& centersFileName, std::vector<std::string>& channels, std::deque< std::vector<RealFeature> >& Bs, const unsigned max = std::numeric_limits<unsigned>::max());

//! Write class centers and their associated channels to a file
bool writeCentersToFile(const std::string& centersFileName, const std::vector<std::string>& channels, const std::vector<RealFeature>& B);

//! Write a list of class centers and their associated channels to a file
bool writeCentersToFile(const std::string& centersFileName, const std::vector<std::string>& channels, const std::deque< std::vector<RealFeature> >& Bs, const unsigned max = std::numeric_limits<unsigned>::max());

//! Read class centers, etas and their associated channels from a file
bool readCentersEtasFromFile(const std::string& centersFileName, std::vector<std::string>& channels, std::vector<RealFeature>& B, std::vector<Real>& Eta);

//! Read a list of class centers, etas and their associated channels from a file
bool readCentersEtasFromFile(const std::string& centersFileName, std::vector<std::string>& channels, std::deque< std::vector<RealFeature> >& Bs, std::deque< std::vector<Real> >& Etas, const unsigned max = std::numeric_limits<unsigned>::max());

//! Write class centers, etas and their associated channels to a file
bool writeCentersEtasToFile(const std::string& centersFileName, const std::vector<std::string>& channels, const std::vector<RealFeature>& B, const std::vector<Real>& Eta);

//! Write a list of class centers, etas and their associated channels to a file
bool writeCentersEtasToFile(const std::string& centersFileName, const std::vector<std::string>& channels, const std::deque< std::vector<RealFeature> >& Bs, const std::deque< std::vector<Real> >& Etas, const unsigned max = std::numeric_limits<unsigned>::max());

//! Read and parse a coordinate from a string
bool readCoordinate(RealPixLoc& coordinate, std::string sCoordinate);

//! Read and creates a EUV image from a fits files name
/*! It will try to guess the Image type if it is UNKNOWN */
EUVImage* getImageFromFile(const std::string imageType, const std::string sunImageFileName);

//! Read and creates a color map from a fits files name
ColorMap* getColorMapFromFile(const std::string sunImageFileName);

//! Return the median class centers of a list of class centers
std::vector<RealFeature> median(const std::deque< std::vector<RealFeature> >& Bs);

//! Return the median class centers of a list of class centers
std::vector<Real> median(const std::deque< std::vector<Real> >& Etas);

//! Reorder the vecort of images according to the channels
bool reorderImages(std::vector<EUVImage*>& images, const std::vector<std::string>& channels);

#endif
