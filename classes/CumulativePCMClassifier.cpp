#include "CumulativePCMClassifier.h"

using namespace std;

CumulativePCMClassifier::CumulativePCMClassifier(Real fuzzifier)
:HistogramPCMClassifier(), CumulativeFCMClassifier()
{
	this->fuzzifier = fuzzifier;
}

CumulativePCMClassifier::CumulativePCMClassifier(const RealFeature& binSize, Real fuzzifier)
:HistogramPCMClassifier(), CumulativeFCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);

}

CumulativePCMClassifier::CumulativePCMClassifier(const std::string& histogramFilename, Real fuzzifier)
:HistogramPCMClassifier(), CumulativeFCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
}
