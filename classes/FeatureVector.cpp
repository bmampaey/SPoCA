#include "FeatureVector.h"

using namespace std;

template<class T, unsigned N>
inline Real d2(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2)
{
	return fv1.d2(fv2);
}


template<class T, unsigned N>
inline Real d(const FeatureVector<T, N>& fv1, const FeatureVector<T, N>& fv2)
{
	return sqrt(d2(fv1,fv2));
}


template<class T, unsigned N>
ostream& operator<<(ostream& out, const FeatureVector<T, N>& fv)
{
	out<<fv.v[0];
	for (unsigned p = 1; p < N; ++p)
		out<<","<<fv.v[p];

	return out;
}


template<class T, unsigned N>
istream& operator>>(istream& in, FeatureVector<T, N>& fv)
{
	char separator;
	in>>fv.v[0];
	for (unsigned p = 1; p < N && in.good(); ++p)
		in>>separator>>fv.v[p];

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


template<class T, unsigned N>
ostream& operator<<(ostream& out, const vector<FeatureVector<T, N> >& v)
{
	if (v.size() > 0)
	{
		out<<v.at(0);
		for (unsigned i = 1 ; i < v.size() ; ++i)
			out<<" "<<v.at(i);

	}
	else
	{
		out<<"Empty vector";
	}

	return out;
}


template<class T, unsigned N>
istream& operator>>(istream& in, vector<FeatureVector <T, N> >& v)
{
	v.resize(0);
	char separator;
	FeatureVector<T, N> fv;
	in>>fv;
	while (in.good())
	{
		v.push_back(fv);
		in>>separator>>fv;
	}
	v.push_back(fv);
	return in;

}

/*! @file FeatureVector.cpp
Instantiation of the template class FeatureVector for Real
See @ref Compilation_Options constants.h */
template class FeatureVector<Real, NUMBERCHANNELS>;

template Real d2<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv1, const FeatureVector<Real, NUMBERCHANNELS>& fv2);
template Real d<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv1, const FeatureVector<Real, NUMBERCHANNELS>& fv2);
template ostream& operator<< <Real, NUMBERCHANNELS>(ostream& out, const FeatureVector<Real, NUMBERCHANNELS>& fv);
template istream& operator>> <Real, NUMBERCHANNELS>(istream& in, FeatureVector<Real, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS> sqrt<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv);
template ostream& operator<< <Real, NUMBERCHANNELS>(ostream& out, const vector<FeatureVector<Real, NUMBERCHANNELS> >& v);
template istream& operator>> <Real, NUMBERCHANNELS>(istream& in, vector<FeatureVector <Real, NUMBERCHANNELS> >& v);
