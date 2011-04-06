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
		mutable unsigned c;

	public :
		HistogramFeatureVector():FeatureVector<T, N>(),c(0){}
		HistogramFeatureVector(T const &value):FeatureVector<T, N>(value),c(0){}
		HistogramFeatureVector(FeatureVector<T, N> const &fv):FeatureVector<T, N>(fv),c(0){}
		HistogramFeatureVector<T,N>& operator++()
		{
			++c;
			return *this;
		}
		
		bool operator<(const HistogramFeatureVector<T, N>& fv) const
		{
			for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
			{
				if(this->v[p] < fv.v[p])
					return true;
				if(this->v[p] > fv.v[p])
					return false;
			}
			return false;
		}

};

typedef HistogramFeatureVector<Real, NUMBERCHANNELS> HistoRealFeature;
typedef HistogramFeatureVector<PixelType, NUMBERCHANNELS> HistoPixelFeature;

template<class T, unsigned N>
inline std::ostream& operator<<(std::ostream& out, const HistogramFeatureVector<T, N>& fv)
{
	out<<fv.v[0];
	for (unsigned p = 1; p < N; ++p)
		out<<","<<fv.v[p];
	out<<" "<<fv.c;
	return out;
}


template<class T, unsigned N>
inline std::istream& operator>>(std::istream& in, HistogramFeatureVector<T, N>& fv)
{
	char separator;
	in>>fv.v[0];
	for (unsigned p = 1; p < N && in.good(); ++p)
		in>>separator>>fv.v[p];
	in>>separator>>fv.c;
	return in;
}

inline int compare(const HistoPixelFeature& x1, const HistoPixelFeature& x2)
{
	for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
	{
		if(x1.v[p] < x2.v[p])
			return -1;
		if(x1.v[p] > x2.v[p])
			return 1;
	}
	return 0;
}



#endif
