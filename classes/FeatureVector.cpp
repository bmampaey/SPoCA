#include "FeatureVector.h"

using namespace std;

template<class T, class T2, unsigned N>
inline Real d2(const FeatureVector<T, N>& pixel, const FeatureVector<T2, N>& centre)
{
	Real d;
	Real sum = 0;
	for (unsigned p = 0; p < N; ++p)
	{
		d = pixel.v[p] - centre.v[p];
		sum += d * d;
	}
	return sum;
}


template<class T, class T2, unsigned N>
inline Real d(const FeatureVector<T, N>& pixel, const FeatureVector<T2, N>& centre)
{
	return sqrt(d2(pixel,centre));
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


/* We create the code for the template class we need
   See constants.h */

template class FeatureVector<Real, NUMBERCHANNELS>;

template Real d2<Real, Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& pixel, const FeatureVector<Real, NUMBERCHANNELS>& centre);
template Real d<Real, Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& pixel, const FeatureVector<Real, NUMBERCHANNELS>& centre);
template ostream& operator<< <Real, NUMBERCHANNELS>(ostream& out, const FeatureVector<Real, NUMBERCHANNELS>& fv);
template istream& operator>> <Real, NUMBERCHANNELS>(istream& in, FeatureVector<Real, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS> sqrt<Real, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& fv);
template ostream& operator<< <Real, NUMBERCHANNELS>(ostream& out, const vector<FeatureVector<Real, NUMBERCHANNELS> >& v);
template istream& operator>> <Real, NUMBERCHANNELS>(istream& in, vector<FeatureVector <Real, NUMBERCHANNELS> >& v);

#if PIXELTYPE!=REAL
template class FeatureVector<PixelType, NUMBERCHANNELS>;

template Real d2<PixelType, Real, NUMBERCHANNELS>(const FeatureVector<PixelType, NUMBERCHANNELS>& pixel, const FeatureVector<Real, NUMBERCHANNELS>& centre);
template Real d2<Real, PixelType, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& pixel, const FeatureVector<PixelType, NUMBERCHANNELS>& centre);
template Real d2<PixelType, PixelType, NUMBERCHANNELS>(const FeatureVector<PixelType, NUMBERCHANNELS>& pixel, const FeatureVector<PixelType, NUMBERCHANNELS>& centre);
template Real d<PixelType, Real, NUMBERCHANNELS>(const FeatureVector<PixelType, NUMBERCHANNELS>& pixel, const FeatureVector<Real, NUMBERCHANNELS>& centre);
template Real d<Real, PixelType, NUMBERCHANNELS>(const FeatureVector<Real, NUMBERCHANNELS>& pixel, const FeatureVector<PixelType, NUMBERCHANNELS>& centre);
template Real d<PixelType, PixelType, NUMBERCHANNELS>(const FeatureVector<PixelType, NUMBERCHANNELS>& pixel, const FeatureVector<PixelType, NUMBERCHANNELS>& centre);
template ostream& operator<< <PixelType, NUMBERCHANNELS>(ostream& out, const FeatureVector<PixelType, NUMBERCHANNELS>& fv);
template istream& operator>> <PixelType, NUMBERCHANNELS>(istream& in, FeatureVector<PixelType, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS> sqrt<PixelType, NUMBERCHANNELS>(const FeatureVector<PixelType, NUMBERCHANNELS>& fv);
template ostream& operator<< <PixelType, NUMBERCHANNELS>(ostream& out, const vector<FeatureVector<PixelType, NUMBERCHANNELS> >& v);
template istream& operator>> <PixelType, NUMBERCHANNELS>(istream& in, vector<FeatureVector <PixelType, NUMBERCHANNELS> >& v);
template FeatureVector<PixelType, NUMBERCHANNELS>& FeatureVector<PixelType, NUMBERCHANNELS>::operator= (const FeatureVector<Real, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS>& FeatureVector<Real, NUMBERCHANNELS>::operator= (const FeatureVector<PixelType, NUMBERCHANNELS>& fv);
template FeatureVector<PixelType, NUMBERCHANNELS>::FeatureVector (const FeatureVector<Real, NUMBERCHANNELS>& fv);
template FeatureVector<Real, NUMBERCHANNELS>::FeatureVector  (const FeatureVector<PixelType, NUMBERCHANNELS>& fv);
#endif
