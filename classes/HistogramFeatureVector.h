#pragma once
#ifndef HistogramFeatureVector_H
#define HistogramFeatureVector_H

#include <iostream>
#include <cmath>
#include <vector>
#include "constants.h"
#include "FeatureVector.h"

template<class T, unsigned N>
class HistogramFeatureVector : public FeatureVector<T, N>
{

	public :
		//The count of the histogramm
		unsigned c;

	public :
		HistogramFeatureVector():FeatureVector<T, N>(),c(0){}
		HistogramFeatureVector(T const &value):FeatureVector<T, N>(value),c(0){}
		HistogramFeatureVector(FeatureVector<T, N> const &fv):FeatureVector<T, N>(fv),c(0){}
		HistogramFeatureVector<T,N>& operator++()
		{
			++c;
			return *this;
		}

};

typedef HistogramFeatureVector<Real, NUMBERWAVELENGTH> HistoRealFeature;
typedef HistogramFeatureVector<PixelType, NUMBERWAVELENGTH> HistoPixelFeature;

inline int compare(const HistoPixelFeature& x1, const HistoPixelFeature& x2)
{
	for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
	{
		if(x1.v[p] < x2.v[p])
			return -1;
		if(x1.v[p] > x2.v[p])
			return 1;
	}
	return 0;
}
#endif
