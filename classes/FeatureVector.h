#pragma once
#ifndef FeatureVector_H
#define FeatureVector_H

#include <iostream>
#include <cmath>
#include <vector>
#include "constants.h"

//! Class FeatureVector
/*!
It is a simple vector of values for the classification.

Some common operations and routines are defined to facilitate the programmation. 
*/


//! @tparam T Type of a a feaure
//! @tparam N Number of features
template<class T, unsigned N>
class FeatureVector
{

	public :
		//! The vector of features
		T v[N];
	public :
		//! Constructor
		FeatureVector(){}
		//! Constructor
		/*! Assign all features to value */
		FeatureVector(T const &value)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = value;
		}
		//! Copy constructor
		FeatureVector(const FeatureVector& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = (T)fv.v[p];
		}
		//! Assignement operator
		FeatureVector& operator=(const FeatureVector& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = (T)fv.v[p];
			return *this;
		}
		//! Square of the euclidian distance to fv
		Real d2(const FeatureVector& fv) const
		{
			Real d;
			Real sum = 0;
			for (unsigned p = 0; p < N; ++p)
			{
				d = (Real)v[p] - (Real)fv.v[p];
				sum += d * d;
			}
			return sum;
		}
		//! Multiply each element by value
		FeatureVector<Real, N> operator*(const Real& value) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = (Real)v[p] * value;
			return result;
		}
		//! Multiplication element by element
		FeatureVector operator*(const FeatureVector& fv) const
		{
			FeatureVector result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] * fv.v[p];
			return result;
		}
		//! Divide each element by value
		FeatureVector<Real, N> operator/(const Real& value) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = (Real)v[p] / value;
			return result;
		}
		//! Division element by element
		FeatureVector<Real, N> operator/(const FeatureVector& fv) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = (Real)v[p] / fv.v[p];
			return result;
		}
		//! Substraction element by element
		FeatureVector operator-(const FeatureVector& fv) const
		{
			FeatureVector result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] - fv.v[p];
			return result;
		}
		//! Addition element by element
		FeatureVector operator+(const FeatureVector& fv) const
		{
			FeatureVector result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] + fv.v[p];
			return result;
		}
		//! Comparison operator
		/*! Compare the modulus of the feature vectors */
		bool operator<(const FeatureVector& fv) const
		{
			FeatureVector zero = 0;
			if(this->d2(zero) < fv.d2(zero))
				return true;
			else
				return false;
		}
		//! Addition element by element
		void operator += (const FeatureVector& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] += fv.v[p];
		}
		//Multiplication element by element
		void operator *= (const FeatureVector& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] *= fv.v[p];
		}
		//! Substraction element by element
		void operator -= (const FeatureVector& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] -= fv.v[p];
		}
		//! Divide each element by value
		void operator /= (Real const &value)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] /= value;
		}
		//! Test if at least one of the feature is zero
		bool has_null() const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] == 0)
					return true;
			}
			return false;
		}
		//! Test if all the features are zero
		bool is_null() const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] != 0)
					return false;
			}
			return true;
		}
		//! Comparison operator
		/*! Compare if the feature vectors are equals */
		bool operator == (const FeatureVector& fv) const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] != fv.v[p])
					return false;
			}
			return true;
		}
		//! Comparison operator
		/*! Compare if the feature vectors are different */
		bool operator != (const FeatureVector& fv) const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] != fv.v[p])
					return true;
			}
			return false;
		}

};

//! Square of the Euclidian distance between 2 feature vectors  
template<class T, unsigned N>
Real d2(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2);

//! Euclidian distance between 2 feature vectors
template<class T, unsigned N>
Real d(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2);

//! Square root of the features of a feature vector
template<class T, unsigned N>
FeatureVector<Real, N> sqrt(const FeatureVector<T, N>& fv);

//! Output of a FeatureVector
template<class T, unsigned N>
std::ostream& operator<<(std::ostream& out, const FeatureVector<T, N>& fv);

//! Input of a FeatureVector
template<class T, unsigned N>
std::istream& operator>>(std::istream& in, FeatureVector<T, N>& fv);

//! Output of a vector of FeatureVector
template<class T, unsigned N>
std::ostream& operator<<(std::ostream& out, const std::vector<FeatureVector<T, N> >& v);

//! Input of a vector of FeatureVector
template<class T, unsigned N>
std::istream& operator>>(std::istream& in, std::vector<FeatureVector<T, N> >& v);

//! Type of the FeatureVector
typedef FeatureVector<Real, NUMBERCHANNELS> RealFeature;
#endif
