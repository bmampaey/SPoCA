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
			out<<"\t"<<v.at(i);

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

template class FeatureVector<Real, NUMBERWAVELENGTH>;

template Real d2<Real, Real, NUMBERWAVELENGTH>(const FeatureVector<Real, NUMBERWAVELENGTH>& pixel, const FeatureVector<Real, NUMBERWAVELENGTH>& centre);
template Real d<Real, Real, NUMBERWAVELENGTH>(const FeatureVector<Real, NUMBERWAVELENGTH>& pixel, const FeatureVector<Real, NUMBERWAVELENGTH>& centre);
template ostream& operator<< <Real, NUMBERWAVELENGTH>(ostream& out, const FeatureVector<Real, NUMBERWAVELENGTH>& fv);
template istream& operator>> <Real, NUMBERWAVELENGTH>(istream& in, FeatureVector<Real, NUMBERWAVELENGTH>& fv);
template FeatureVector<Real, NUMBERWAVELENGTH> sqrt<Real, NUMBERWAVELENGTH>(const FeatureVector<Real, NUMBERWAVELENGTH>& fv);
template ostream& operator<< <Real, NUMBERWAVELENGTH>(ostream& out, const vector<FeatureVector<Real, NUMBERWAVELENGTH> >& v);
template istream& operator>> <Real, NUMBERWAVELENGTH>(istream& in, vector<FeatureVector <Real, NUMBERWAVELENGTH> >& v);

#if PIXELTYPE!=REALTYPE
template class FeatureVector<PixelType, NUMBERWAVELENGTH>;

template Real d2<PixelType, Real, NUMBERWAVELENGTH>(const FeatureVector<PixelType, NUMBERWAVELENGTH>& pixel, const FeatureVector<Real, NUMBERWAVELENGTH>& centre);
template Real d2<Real, PixelType, NUMBERWAVELENGTH>(const FeatureVector<Real, NUMBERWAVELENGTH>& pixel, const FeatureVector<PixelType, NUMBERWAVELENGTH>& centre);
template Real d2<PixelType, PixelType, NUMBERWAVELENGTH>(const FeatureVector<PixelType, NUMBERWAVELENGTH>& pixel, const FeatureVector<PixelType, NUMBERWAVELENGTH>& centre);
template Real d<PixelType, Real, NUMBERWAVELENGTH>(const FeatureVector<PixelType, NUMBERWAVELENGTH>& pixel, const FeatureVector<Real, NUMBERWAVELENGTH>& centre);
template Real d<Real, PixelType, NUMBERWAVELENGTH>(const FeatureVector<Real, NUMBERWAVELENGTH>& pixel, const FeatureVector<PixelType, NUMBERWAVELENGTH>& centre);
template Real d<PixelType, PixelType, NUMBERWAVELENGTH>(const FeatureVector<PixelType, NUMBERWAVELENGTH>& pixel, const FeatureVector<PixelType, NUMBERWAVELENGTH>& centre);
template ostream& operator<< <PixelType, NUMBERWAVELENGTH>(ostream& out, const FeatureVector<PixelType, NUMBERWAVELENGTH>& fv);
template istream& operator>> <PixelType, NUMBERWAVELENGTH>(istream& in, FeatureVector<PixelType, NUMBERWAVELENGTH>& fv);
template FeatureVector<Real, NUMBERWAVELENGTH> sqrt<PixelType, NUMBERWAVELENGTH>(const FeatureVector<PixelType, NUMBERWAVELENGTH>& fv);
template ostream& operator<< <PixelType, NUMBERWAVELENGTH>(ostream& out, const vector<FeatureVector<PixelType, NUMBERWAVELENGTH> >& v);
template istream& operator>> <PixelType, NUMBERWAVELENGTH>(istream& in, vector<FeatureVector <PixelType, NUMBERWAVELENGTH> >& v);
template FeatureVector<PixelType, NUMBERWAVELENGTH>& FeatureVector<PixelType, NUMBERWAVELENGTH>::operator= (const FeatureVector<Real, NUMBERWAVELENGTH>& fv);
template FeatureVector<Real, NUMBERWAVELENGTH>& FeatureVector<Real, NUMBERWAVELENGTH>::operator= (const FeatureVector<PixelType, NUMBERWAVELENGTH>& fv);
template FeatureVector<PixelType, NUMBERWAVELENGTH>::FeatureVector (const FeatureVector<Real, NUMBERWAVELENGTH>& fv);
template FeatureVector<Real, NUMBERWAVELENGTH>::FeatureVector  (const FeatureVector<PixelType, NUMBERWAVELENGTH>& fv);
#endif
