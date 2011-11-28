#include "Coordinate.h"

using namespace std;

/*! @file Coordinate.cpp
Instantiation of the template class CartesianCoordinate for unsigned
*/
//template class CartesianCoordinate<unsigned>;

/*! @file Coordinate.cpp
Instantiation of the template class CartesianCoordinate for Real
*/
//template class CartesianCoordinate<Real>;

template <class T>
ostream& operator<<(ostream& out, const CartesianCoordinate<T>& c)
{
	out<<"("<<c.x<<","<<c.y<<")";
	return out;
}

template <class T>
istream& operator>>(istream& in, CartesianCoordinate<T>& c)
{
	char separator;
	in>>separator>>c.x>>separator>>c.y>>separator;
	return in;
}

template ostream& operator<<(ostream& out, const CartesianCoordinate<Real>& c);
template ostream& operator<<(ostream& out, const CartesianCoordinate<unsigned>& c);

template istream& operator>>(istream& in, CartesianCoordinate<Real>& c);
template istream& operator>>(istream& in, CartesianCoordinate<unsigned>& c);

template <class T>
inline Real distance(const CartesianCoordinate<T>& a, const CartesianCoordinate<T>& b)
{
	return sqrt(distance_squared(a, b));
}

template <class T>
inline Real distance_squared(const CartesianCoordinate<T>& a, const CartesianCoordinate<T>& b)
{
	Real dx = (a.x - b.x);
	Real dy = (a.y - b.y);
	return  dx * dx + dy * dy;
}

template Real distance(const CartesianCoordinate<Real>& a, const CartesianCoordinate<Real>& b);
template Real distance(const CartesianCoordinate<unsigned>& a, const CartesianCoordinate<unsigned>& b);

template Real distance_squared(const CartesianCoordinate<Real>& a, const CartesianCoordinate<Real>& b);
template Real distance_squared(const CartesianCoordinate<unsigned>& a, const CartesianCoordinate<unsigned>& b);
/*
ostream& operator<<(ostream& out, const RealPixLoc& c)
{
	out<<"("<<c.x<<","<<c.y<<")";
	return out;
}

istream& operator>>(istream& in, RealPixLoc& c)
{
	char separator;
	in>>separator>>c.x>>separator>>c.y>>separator;
	return in;
}
*/
ostream& operator<<(ostream& out, const HCC& c)
{
	out<<"("<<c.x<<","<<c.y<<","<<c.z<<")";
	return out;
}

istream& operator>>(istream& in, HCC& c)
{
	char separator;
	in>>separator>>c.x>>separator>>c.y>>separator>>c.z>>separator;
	return in;
}





ostream& operator<<(ostream& out, const HeliographicCoordinate& c)
{
	out<<"("<<c.longitude*RADIAN2DEGREE<<","<<c.latitude*RADIAN2DEGREE<<")";
	return out;
}

ostream& operator<<(ostream& out, HeliographicCoordinate& c)
{
	out<<"("<<c.longitude<<","<<c.latitude<<")";
	c.longitude *= DEGREE2RADIAN;
	c.latitude *= DEGREE2RADIAN;
	return out;
}

inline Real distance(const HeliographicCoordinate& a, const HeliographicCoordinate& b, const Real& R)
{
	return acos(sin(a.latitude)*sin(b.latitude)+cos(a.latitude)*cos(b.latitude)*cos(a.longitude-b.longitude))*R;
}

inline Real distance_squared(const HeliographicCoordinate& a, const HeliographicCoordinate& b, const Real& R)
{
	Real d = distance(a, b, R);
	return d*d;
}

