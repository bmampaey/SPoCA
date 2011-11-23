
#ifndef HistogramClassifier_H
#define HistogramClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <fstream>
#include <fenv.h>
#include <iomanip>
#include <set>


#include "EUVImage.h"
#include "HistogramFeatureVector.h"
#include "FeatureVector.h"

//! Base class of all histogram classifier classes
/*!
The histogram based classification is an optimisation of the regular classification.
It consist in making a histogram of the FeatureVectors and run the classification algorithm on that histogram.
Although the results will vary somewhat from the regular classification, if the bin size is well chosen, they will be very close.

It is only possible to do histogram classification for non Spatial classifiers, as the histogram looses the information about the location of the pixels.

For the same reason, it is not possible to do segmentation with the histogram classifiers. It is neccessary to use the centers found and do an attribution with a regular classifier.

The class is purely virtual as it does not define any classification method itself.
*/

//! The type for the set of histogram feature vectors
typedef std::set<HistoRealFeature> HistoFeatureVectorSet;

//! The type for the set of feature vectors
typedef std::vector<RealFeature> FeatureVectorSet;

class HistogramClassifier
{
	protected :
	
		//! Set of the histogram feature vector
		HistoFeatureVectorSet HistoX;
		
		//! Size of the bin of the histogram
		RealFeature binSize;
		
		//! Number of the bin of the histogram
		unsigned numberBins;
		
		//! Channels of the histogram
		std::vector<std::string> histoChannels;

	protected :
		//! Function to insert a new HistoFeatureVector into HistoX
		void insert(const HistoRealFeature& xj);

		//! Function to insert a new FeatureVector into HistoX
		void insert(const RealFeature& xj);

	public :
		//! Constructor
		HistogramClassifier();
		//! Constructor
		HistogramClassifier(const RealFeature& binSize);
		//! Constructor
		HistogramClassifier(const std::string& histogramFilename);
		//! Destructor
		virtual ~HistogramClassifier(){}
		
		//! Routine to initialise the bin size
		void initBinSize(const RealFeature& binSize);
		
		//! Routine to initialise the histogram
		/*! @param reset If true, creates the bin with a value of 0 */
		void initHistogram(const std::string& histogramFilename, bool reset = true);
		
		//! Routine to save the histogram to a file
		void saveHistogram(const std::string& histogramFilename);
		
		//! Routine to add images to the histogram
		virtual void addImages(std::vector<EUVImage*> images, const unsigned xaxes, const unsigned yaxes);
		
		//! Routine to add a vector of FeatureVector to the histogram
		virtual void addFeatures(const FeatureVectorSet& X);
};
#endif
