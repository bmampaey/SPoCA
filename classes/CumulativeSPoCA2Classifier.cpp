#include "CumulativeSPoCA2Classifier.h"

using namespace std;

CumulativeSPoCA2Classifier::CumulativeSPoCA2Classifier(unsigned neighboorhoodRadius, Real fuzzifier)
:SPoCA2Classifier(), CumulativeSPoCAClassifier()
{
	this->fuzzifier = fuzzifier;
	this->Nradius = neighboorhoodRadius;
}
