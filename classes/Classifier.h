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
#include "EUVImage.h"
#include "ColorMap.h"
#include "FeatureVector.h"
#include "Region.h"
#include "Coordinate.h"

//! Base class of all classifier classes
/*!
All classification methods are implemented as classes that derive from this class.

The class is purely virtual as it does not define any classification method itself.
*/

class Classifier
{
	protected :
		//! Number of classes
		unsigned numberClasses;
		
		//! Number of feature vectors
		unsigned numberValidPixels;
		
		//! Size of the axes
		unsigned Xaxes, Yaxes;
		
		//! Feature vector descibing the channels (usually the wavelength)
		RealFeature channels;

		//! Vector of membership/probability
		std::vector<Real> U;

		//! Vector of the centers of the classes
		std::vector<RealFeature> B;

		//! Vector of the feature vectors (pixel intensities for example)
		std::vector<PixelFeature> X;

		//! The coordinates of the feature vectors (needed to output the results)
		std::vector<Coordinate> coordinates;

		//! Computation of the centers of classes
		virtual void computeB() = 0;
		
		//! Computation of the membership/probability
		virtual void computeU() = 0;

		//! Computation of J the total intracluster variance
		virtual Real computeJ() const = 0;

		//! Asses function for the sursegmentation
		virtual Real assess(std::vector<Real>& V) = 0;
		
		//! Merge function for the sursegmentation
		virtual void merge(unsigned i1, unsigned i2);
		
		//! Function to initialise the centers
		virtual void initB(const std::vector<RealFeature>& B);

		//! File stream to output the classification steps
		std::ofstream stepfile;
		
		//! Function to initialize the output of the classification step
		virtual void stepinit(const std::string filename);
		
		//! Function to output a classification step
		virtual void stepout(const unsigned iteration, const Real precisionReached, const int precision = 5);

		

	public :
	
		//! Constructor
		Classifier();
		
		//! Destructor
		virtual ~Classifier();

		//! Function to check that the images passed to the Classifier are similar
		void checkImages(const std::vector<EUVImage*>& images);
		
		//! Function to ordonate the images according to the channels
		void ordonateImages(std::vector<EUVImage*>& images);
		
		//! Function to add images to the classifier
		virtual void addImages (std::vector<EUVImage*> images);

		//! Function to do the classification
		virtual void classification(Real precision = 1., unsigned maxNumberIteration = 100) = 0;
		
		//! Function to do attribution (Fix center classification).
		virtual void attribution();

		//! Function to initialise the centers of classes
		virtual void initB(const std::vector<RealFeature>& B, const RealFeature& channels);
		
		//! Function to randomly initialise the centers of classes
		virtual void randomInitB(unsigned C);
		
		//! Function to segment by selection of the class that has the maximal value of membership/probability
		virtual ColorMap* segmentedMap_maxUij(ColorMap* segmentedMap = NULL);
		
		//! Function to segment by selection of the class that has is the closest to the feature vector
		virtual ColorMap* segmentedMap_closestCenter(ColorMap* segmentedMap = NULL);
		
		//! Function to segment by selection of one class and that has membership/probability value between 2 tresholds
		virtual ColorMap* segmentedMap_classTreshold(unsigned i, Real lowerIntensity_minMembership, Real higherIntensity_minMembership, ColorMap* segmentedMap = NULL);
		
		//! Function to segment by using the maxUij segmentation, and grouping them further by their class value
		virtual ColorMap* segmentedMap_limits(std::vector<RealFeature>& limits, ColorMap* segmentedMap = NULL);
		
		//! Function to segment by using the maxUij segmentation, and grouping them further by their class number
		virtual ColorMap* segmentedMap_fixed(std::vector<unsigned>& ch, std::vector<unsigned>& qs, std::vector<unsigned>& ar, ColorMap* segmentedMap = NULL);
		
		//! Function that returns the map of membership/probability for a class
		virtual EUVImage* fuzzyMap(const unsigned i, EUVImage* fuzzyMap = NULL);
		
		//! Function that returns the normalized membership/probability for a class
		virtual EUVImage* normalizedFuzzyMap(const unsigned i, EUVImage* fuzzyMap = NULL);
		
		//! Function to do sursegmentation
		unsigned sursegmentation(std::vector<RealFeature>& B, unsigned C = 0);
		
		//! Function to do sursegmentation
		unsigned sursegmentation(unsigned C = 0);

		//! Function to save the centers of classes to a file
		void saveB(const std::string& filename);
		
		//! Function to compute the class average
		virtual std::vector<RealFeature> classAverage() const;
		
		//! Accessor to retrieve the image corresponding to a specific channel 
		EUVImage* getImage(unsigned p);
		
		//! Accessor to retrieve the centers of classes
		std::vector<RealFeature> getB();
		
		//! Accessor to retrieve the channels
		RealFeature getChannels();
		
		//! Accessor to retrieve the percentiles of the feature vectors
		std::vector<PixelFeature> percentiles(std::vector<Real> percentileValues);

};

extern std::string outputFileName;
#endif
