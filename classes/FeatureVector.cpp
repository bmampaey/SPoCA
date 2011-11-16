#include "FeatureVector.h"

using namespace std;

template<class T, unsigned N>
inline Real distance_squared(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2)
{
	if(N == 1)
	{
		return (fv1.v[0] - fv2.v[0]) * (fv1.v[0] - fv2.v[0]);
	}
	else
	{
		Real sum = 0;
		for (unsigned p = 0; p < N; ++p)
		{
			Real d = (Real)fv1.v[p] - (Real)fv2.v[p];
			sum += d * d;
		}
		return sum;
	}
}


template<class T, unsigned N>
inline Real distance(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2)
{
	if(N == 1)
	{
		return fabs(fv1.v[0] - fv2.v[0]);
	}
	else
	{
		return sqrt(distance_squared(fv1,fv2));
	}
}


template<class T, unsigned N>
inline Real norm(const FeatureVector<T, N>& fv)
{
	if(N == 1)
	{
		return fabs(fv.v[0]);
	}
	else
	{
		Real sum = 0;
		for (unsigned p = 0; p < N; ++p)
		{
			sum += fv.v[p] * fv.v[p];
		}
		return sqrt(sum);
	}
}


template<class T, unsigned N>
ostream& operator<<(ostream& out, const FeatureVector<T, N>& fv)
{
	out<<fv.toString(out.precision());
	return out;
}


template<class T, unsigned N>
istream& operator>>(istream& in, FeatureVector<T, N>& fv)
{
	char separator;
	while(in.good() && isspace(char(in.peek())))
	{
		in.get();
	}
	if(! in.good())
	{
		cerr<<"Error parsing FeatureVector from stream"<<endl;
		return in;
	}
	
	bool get_last = in.peek() == '(';
	
	if(get_last)
	{
		in>>separator;
	}
	
	in>>fv.v[0];
	for (unsigned p = 1; p < N && in.good(); ++p)
		in>>separator>>fv.v[p];
	
	if (get_last)
	{
		in>>separator;
	}
	
	return in;
}


template<class T, unsigned N>
inline FeatureVector<Real, N> sqrt(const FeatureVector<T, N>& fv)
{
	FeatureVector<Real, N> result;
	for (unsigned p = 0; p < N; ++p)
		result.v[p] = sqrt(fv.v[p]);
	return result;
}



/*! @file FeatureVector.cpp
Instantiation of the template class FeatureVector for Real
See @ref Compilation_Options constants.h */
template class FeatureVector<Real, NUMBERCHANNELS>;

template Real distance_squared<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv1, const FeatureVector<Real, NUMBERCHANNELS>& fv2);
template Real distance<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv1, const FeatureVector<Real, NUMBERCHANNELS>& fv2);
template Real norm<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv);
template ostream& operator<< <Real, NUMBERCHANNELS>(ostream& out, const FeatureVector<Real, NUMBERCHANNELS>& fv);
template istream& operator>> <Real, NUMBERCHANNELS>(istream& in, FeatureVector<Real, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS> sqrt<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv);
