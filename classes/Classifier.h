#pragma once
#ifndef Classifier_H
#define Classifier_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <fenv.h>
#include <iomanip>


#include "tools.h"
#include "constants.h"
#include "Image.h"
#include "SunImage.h"
#include "FeatureVector.h"
#include "Region.h"
#include "Coordinate.h"

class Classifier
{
	protected :
		unsigned    numberClasses;
		unsigned    numberValidPixels;
		unsigned    Xaxes, Yaxes;
		
		//Feature vector descibing the channels (usually the wavelength)
		RealFeature channels;

		//vector of membership
		std::vector<Real> U;

		//vector of centers
		std::vector<RealFeature> B;

		//vector of feature vectors (pixel intensities for example)
		std::vector<PixelFeature> X;

		//The coordinates of the feature vectors (needed to output the results)
		std::vector<Coordinate> coordinates;

		//Computation of centers and membership
		virtual void computeB() = 0;
		virtual void computeU() = 0;

		//Computation of J
		virtual Real computeJ() const = 0;

		//Asses & Merge functions for the sursegmentation
		virtual Real assess(std::vector<Real>& V) = 0;
		virtual void merge(unsigned i1, unsigned i2);
		
		//Function to initialise the centers
		virtual void initB(const std::vector<RealFeature>& B);
		

		//File stream to output classification steps
		std::ofstream stepfile;
		
		//Function to output classification steps
		virtual void stepinit(const std::string filename);
		virtual void stepout(const unsigned iteration, const Real precisionReached, const int precision = 5);

		

	public :
	
		//Constructors & Destructors
		Classifier();
		virtual ~Classifier();

		//Functions to add and check images
		void checkImages(const std::vector<SunImage*>& images);
		void ordonateImages(std::vector<SunImage*>& images);
		virtual void addImages (std::vector<SunImage*>& images);

		//Classification functions
		virtual void classification(Real precision = 1., unsigned maxNumberIteration = 100) = 0;
		void attribution();

		//Function to initialise the centers
		virtual void initB(const std::vector<RealFeature>& B, const RealFeature& channels);
		virtual void randomInitB(unsigned C);
		
		//Segmentation functions
		virtual Image<unsigned>* segmentedMap_maxUij();
		virtual Image<unsigned>* segmentedMap_closestCenter();
		virtual Image<unsigned>* segmentedMap_classTreshold(unsigned i, Real lowerIntensity_minMembership, Real higherIntensity_minMembership);
		virtual Image<unsigned>* segmentedMap_limits(std::vector<RealFeature>& limits);
		virtual Image<unsigned>* segmentedMap_fixed(std::vector<unsigned>& ch, std::vector<unsigned>& qs, std::vector<unsigned>& ar);
		virtual Image<Real>* fuzzyMap(const unsigned i);
		virtual Image<Real>* normalizedFuzzyMap(const unsigned i);
		
		//Sursegmentation functions
		unsigned sursegmentation(std::vector<RealFeature>& B, unsigned C = 0);
		unsigned sursegmentation(unsigned C = 0);

		//Utilities function for outputing results
		virtual void saveAllResults(SunImage* outImage);
		virtual void saveARmap(SunImage* outImage);
		virtual void saveCHmap(SunImage* outImage);	
		void saveB(const std::string& filename);
		virtual std::vector<RealFeature> classAverage() const;
		
		//Accessors
		SunImage* getImage(unsigned p);
		std::vector<RealFeature> getB();
		std::vector<PixelFeature> percentiles(std::vector<Real> percentileValues);

};

extern std::string outputFileName;
#endif
