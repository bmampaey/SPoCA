#include "CumulativePCM2Classifier.h"

using namespace std;

CumulativePCM2Classifier::CumulativePCM2Classifier(Real fuzzifier)
:HistogramPCM2Classifier(), CumulativePCMClassifier()
{
	this->fuzzifier = fuzzifier;
}

CumulativePCM2Classifier::CumulativePCM2Classifier(const RealFeature& binSize, Real fuzzifier)
:HistogramPCM2Classifier(), CumulativePCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);

}

CumulativePCM2Classifier::CumulativePCM2Classifier(const std::string& histogramFilename, Real fuzzifier)
:HistogramPCM2Classifier(), CumulativePCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
}
