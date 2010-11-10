#pragma once
#ifndef FeatureVector_H
#define FeatureVector_H

#include <iostream>
#include <cmath>
#include <vector>
#include "constants.h"

template<class T, unsigned N>
class FeatureVector
{

	public :

		T v[N];

		FeatureVector(){}
		FeatureVector(T const &value)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = value;
		}
		template<class T2>
			FeatureVector<T, N>& operator=(const FeatureVector<T2, N>& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = (T)fv.v[p];
			return *this;
		}
		template<class T2>
			FeatureVector(const FeatureVector<T2, N>& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = (T)fv.v[p];
		}
		void operator=(T const &value)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] = value;
		}
		Real d2(const FeatureVector<T, N>& centre) const
		{
			Real d;
			Real sum = 0;
			for (unsigned p = 0; p < N; ++p)
			{
				d = v[p] - centre.v[p];
				sum += d * d;
			}
			return sum;
		}
		FeatureVector<Real, N> operator*(const Real& value) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] * value;
			return result;
		}
		//Multiplication element by element
		FeatureVector<T, N> operator*(const FeatureVector<T, N>& fv) const
		{
			FeatureVector<T, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] * fv.v[p];
			return result;
		}
		FeatureVector<Real, N> operator/(const Real& value) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] / value;
			return result;
		}
		//Division element by element
		FeatureVector<Real, N> operator/(const FeatureVector<T, N>& fv) const
		{
			FeatureVector<Real, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] / fv.v[p];
			return result;
		}

		FeatureVector<T, N> operator-(const FeatureVector<T, N>& fv) const
		{
			FeatureVector<T, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] - fv.v[p];
			return result;
		}
		FeatureVector<T, N> operator+(const FeatureVector<T, N>& fv) const
		{
			FeatureVector<T, N> result;
			for (unsigned p = 0; p < N; ++p)
				result.v[p] = v[p] + fv.v[p];
			return result;
		}
		bool operator<(const FeatureVector<T, N>& fv) const
		{
			FeatureVector<T, N> zero = 0;
			if(this->d2(zero) < fv.d2(zero))
				return true;
			else
				return false;
		}

		void operator += (const FeatureVector<T, N>& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] += fv.v[p];
		}
		//Multiplication element by element
		void operator *= (const FeatureVector<T, N>& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] *= fv.v[p];
		}
		void operator -= (const FeatureVector<T, N>& fv)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] -= fv.v[p];
		}
		void operator /= (Real const &value)
		{
			for (unsigned p = 0; p < N; ++p)
				v[p] /= value;
		}
		operator bool() const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p])
					return true;
			}
			return false;
		}
		bool operator == (const FeatureVector<T, N>& fv) const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] != fv.v[p])
					return false;
			}
			return true;
		}
		bool operator != (const FeatureVector<T, N>& fv) const
		{
			for (unsigned p = 0; p < N; ++p)
			{
				if(v[p] != fv.v[p])
					return true;
			}
			return false;
		}

};

//Euclidian distance between 2 FeatureVector  
template<class T, class T2, unsigned N>
Real d2(const FeatureVector<T, N>& pixel, const FeatureVector<T2, N>& centre);
template<class T, class T2, unsigned N>
Real d(const FeatureVector<T, N>& pixel, const FeatureVector<T2, N>& centre);

//Sqrt of a FeatureVector element by element
template<class T, unsigned N>
FeatureVector<Real, N> sqrt(const FeatureVector<T, N>& fv);

//Input\Output of a FeatureVector
template<class T, unsigned N>
std::ostream& operator<<(std::ostream& out, const FeatureVector<T, N>& fv);
template<class T, unsigned N>
std::istream& operator>>(std::istream& in, FeatureVector<T, N>& fv);

//Input\Output of a vector of FeatureVector
template<class T, unsigned N>
std::ostream& operator<<(std::ostream& out, const std::vector<FeatureVector<T, N> >& v);
template<class T, unsigned N>
std::istream& operator>>(std::istream& in, std::vector<FeatureVector<T, N> >& v);

typedef FeatureVector<Real, NUMBERWAVELENGTH> RealFeature;
typedef FeatureVector<PixelType, NUMBERWAVELENGTH> PixelFeature;
#endif
