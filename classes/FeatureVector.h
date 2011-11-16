#pragma once
#ifndef FeatureVector_H
#define FeatureVector_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>

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
		//! Destructor
		virtual ~FeatureVector(){}
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
		/*! Compare the norm of the feature vectors */
		bool operator<(const FeatureVector& fv) const
		{
			if(distance_squared(*this, FeatureVector<T, N>(0)) < distance_squared(fv, FeatureVector<T, N>(0)))
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
		
		virtual std::string toString(const unsigned& precision = 0) const
		{
			std::ostringstream out;
			if (precision != 0)
				out<<std::fixed<<std::setprecision(precision)<<"("<<v[0];
			else
				out<<std::fixed<<std::noshowpoint<<"("<<v[0];
			for (unsigned p = 1; p < N; ++p)
				out<<","<<v[p];
			out<<")";
			return out.str();
		}

};

//! Square of the Euclidian distance between 2 feature vectors  
template<class T, unsigned N>
Real distance_squared(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2);

//! Euclidian distance between 2 feature vectors
template<class T, unsigned N>
Real distance(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2);

//! Norm of the feature vector
template<class T, unsigned N>
Real norm(const FeatureVector<T, N>& fv);

//! Square root of the features of a feature vector
template<class T, unsigned N>
FeatureVector<Real, N> sqrt(const FeatureVector<T, N>& fv);

//! Output of a FeatureVector
template<class T, unsigned N>
std::ostream& operator<<(std::ostream& out, const FeatureVector<T, N>& fv);

//! Input of a FeatureVector
template<class T, unsigned N>
std::istream& operator>>(std::istream& in, FeatureVector<T, N>& fv);

//! Type of the FeatureVector
typedef FeatureVector<Real, NUMBERCHANNELS> RealFeature;
#endif
