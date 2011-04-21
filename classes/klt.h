#pragma once
#ifndef KLT_H
#define KLT_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include "SunImage.h"

enum KLT_STATUS {KLT_SMALL_DET, KLT_OOB, KLT_LARGE_RESIDUE, KLT_MAX_ITERATIONS, KLT_TRACKED};

void KLTTrackFeatures(
const SunImage* img1, const SunImage* img2,
std::vector<Coordinate>& featurelist,
int window_width = 28, int window_height = 28,
unsigned pyramidLevels = 4, int subsampling = 2,
float pyramid_sigma_fact = 0.9f,
float grad_sigma = 1.0f,
float smooth_sigma_fact = 0.1f,
int max_iterations = 10,
float max_residue = 10.0f,
float step_factor = 1.0f,
float min_determinant = 0.01f,
float min_displacement = 0.1f);

typedef std::vector<SunImage*> Pyramid;
extern std::string filenamePrefix;
#endif
