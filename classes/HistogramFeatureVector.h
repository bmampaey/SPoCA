#pragma once
#ifndef HistogramFeatureVector_H
#define HistogramFeatureVector_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>

#include "constants.h"
#include "FeatureVector.h"

//! Class HistogramFeatureVector
/*!
This class is needed for the HistogramClassifiers. 

It augments the FeatureVector class with a count for the number of elements in the bin.

Some common operations and routines are defined to facilitate the programmation. 
*/

//! @tparam T Type of a a feaure
//! @tparam N Number of features
template<class T, unsigned N>
class HistogramFeatureVector : public FeatureVector<T, N>
{

	public :
		//! The number of elements in a bin
		/*! It must be declared mutable so that it can be modified when it is stored in a std::set */
		mutable unsigned c;

	public :
		//! Constructor
		HistogramFeatureVector()
		:FeatureVector<T, N>(),c(0){}
		
		//! Destructor
		virtual ~HistogramFeatureVector(){}
		
		//! Constructor
		/*! Assign all features to value */
		HistogramFeatureVector(T const &value)
		:FeatureVector<T, N>(value),c(0){}
		
		//! Copy constructor
		HistogramFeatureVector(FeatureVector<T, N> const &fv)
		:FeatureVector<T, N>(fv),c(0){}
		
		//! Increment the number of elements by 1
		HistogramFeatureVector<T,N>& operator++()
		{
			++c;
			return *this;
		}
		
		//! Comparison operator
		/*! Compare the feature vectors element by element */
		bool operator<(const HistogramFeatureVector& fv) const
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
		
		std::string toString(const unsigned& precision = 0) const
		{
			return FeatureVector<T, N>::toString(precision) + "x" + itos(c);
		}

};

//! Type of the HistogramFeatureVector
typedef HistogramFeatureVector<Real, NUMBERCHANNELS> HistoRealFeature;

//! Output of a HistogramFeatureVector
template<class T, unsigned N>
inline std::ostream& operator<<(std::ostream& out, const HistogramFeatureVector<T, N>& fv)
{
	out<<fv.toString(out.precision());
	return out;
}

//! Input of a HistogramFeatureVector
template<class T, unsigned N>
inline std::istream& operator>>(std::istream& in, HistogramFeatureVector<T, N>& fv)
{
	char separator;
	in>>separator>>fv.v[0];
	for (unsigned p = 1; p < N && in.good(); ++p)
		in>>separator>>fv.v[p];
	in>>separator>>separator>>fv.c;
	return in;
}

/*! Compare the feature vectors element by element */
inline int compare(const HistoRealFeature& x1, const HistoRealFeature& x2)
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
