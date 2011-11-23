// Functions to help main programs

#pragma once
#ifndef MainUtilities_H
#define MainUtilities_H

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sys/stat.h>

#include "FeatureVector.h"
#include "EUVImage.h"
#include "EITImage.h"
#include "EUVIImage.h"
#include "AIAImage.h"
#include "SWAPImage.h"
#include "HMIImage.h"
#include "ColorMap.h"
#include "Header.h"

extern std::string filenamePrefix;
extern const char * instruments[];
extern const char * limb_corrections[];

//! Read class centers and their associated channels from a file
unsigned readCentersFromFile(std::vector<RealFeature>& B, std::vector<std::string>& channels, const std::string& centersFileName);
//! Read a list of class centers and their associated channels from a file
unsigned readCentersFromFile(std::vector< std::vector<RealFeature> >& Bs, std::vector<std::string>& channels, const std::string& centersFileName);

//! Write class centers and their associated channels from a file
void writeCentersToFile(const std::vector<RealFeature>& B, const std::vector<std::string>& channels, const std::string& centersFileName);
//! Write a list of class centers and their associated channels from a file
void writeCentersToFile(const std::vector< std::vector<RealFeature> >& Bs, const std::vector<std::string>& channels, const std::string& centersFileName);

//! Read and parse the binsize from a string
bool readbinSize(RealFeature& binSize, std::string sbinSize);

//! Read and creates one EUV image from a fits files name
/*! It will try to guess the Image type if it is UNKNOWN */
EUVImage* getImageFromFile(const std::string imageType, const std::string sunImageFileName);

//! Read and creates EUV images from fits files names
/*! It will try to guess the Image type if it is UNKNOWN
    @param align Specify if you want all images to be recenterd on the sun center of the first image */
std::vector<EUVImage*> getImagesFromFiles(const std::string imageType, const std::vector<std::string>& sunImagesFileNames, bool align = false);

//! Read and creates one color map from a fits files name
ColorMap* getImageFromFile(const std::string sunImageFileName);

//! Read and parse the eta from a file
unsigned readEtaFromFile(std::vector<Real>& eta, const std::string& etaFileName);

//! Read and parse the max limits from file
unsigned readMaxLimitsFromFile (std::vector<RealFeature>& maxLimits, const std::string& maxLimitsFileName);

//! Expand the text repacing all keywords between {} by their value in the header
std::string expand(std::string text, const Header& header);

//! Return the median class centers of a list of class centers
std::vector<RealFeature> median_classcenters(const std::vector< std::vector<RealFeature> >& Bs);

#endif
