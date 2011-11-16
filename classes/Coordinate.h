#pragma once
#ifndef CartesianCoordinate_H
#define CartesianCoordinate_H

#include <stdio.h> 
#include <limits>
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>

#include "constants.h"

//! General Type for cartesian coordinate
template<class T>
class CartesianCoordinate
{
	public :
		T x, y;

	public :
		//! Constructor
		CartesianCoordinate(const T& x = 0, const T& y = 0):x(x), y(y){};
		
		//! Destructor
		virtual ~CartesianCoordinate(){};
		
		//! Comparison operator
		bool operator==(const CartesianCoordinate& c)const
		{
			if(std::numeric_limits<T>::is_integer)
				return x == c.x && y == c.y;
			else
				return fabs(x - c.x) < std::numeric_limits<T>::epsilon() && fabs(y - c.y) < std::numeric_limits<T>::epsilon();
		}
		/*
		//! Comparison operator
		bool operator!=(const CartesianCoordinate& c)const
		{
			return x != c.x || y != c.y;
		}
		
		//! Comparison operator
		bool operator<(const CartesianCoordinate& c)const
		{
			if (y < c.y)
				return true;
			else if (y > c.y)
				return false;
			else if (x < c.x)
				return true;
			else
				return false;
		}
		
		CartesianCoordinate& operator+=(const CartesianCoordinate& c)
		{
			x += c.x;
			y += c.y;
			return *this;
		}
		CartesianCoordinate& operator-=(const CartesianCoordinate& c)
		{
			x -= c.x;
			y -= c.y;
			return *this;
		}
		CartesianCoordinate operator+(const CartesianCoordinate& c) const
		{
			CartesianCoordinate result(*this);
			return result+=(c);
		}
		CartesianCoordinate operator-(const CartesianCoordinate& c) const
		{
			CartesianCoordinate result(*this);
			return result-=(c);
		}
		
		CartesianCoordinate operator*(const T& value) const
		{
			return CartesianCoordinate(x * value, y * value);
		}
		CartesianCoordinate operator/(const T& value) const
		{
			return CartesianCoordinate(x / value, y / value);
		}
		*/
		
		//! Test if coordinate is null
		virtual bool operator !() const
		{
			if (std::numeric_limits<T>::has_infinity)
				return isinf(x) || isinf(y);
			else
				return x == std::numeric_limits<T>::max() || y == std::numeric_limits<T>::max();
			
		}
		
	public :
		//! The null coordinate
		static const CartesianCoordinate null()
		{
			if (std::numeric_limits<T>::has_infinity)
				return CartesianCoordinate(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());
			else
				return CartesianCoordinate(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
		}

};

//! Euclidian distance between 2 coordinates
template <class T>
Real distance(const CartesianCoordinate<T>& a, const CartesianCoordinate<T>& b);

//! Square of the euclidian distance between 2 coordinates
template <class T>
Real distance_squared(const CartesianCoordinate<T>& a, const CartesianCoordinate<T>& b);

//! Output operator
template <class T>
std::ostream& operator<<(std::ostream& out, const CartesianCoordinate<T>& c);

//! Input operator
template <class T>
std::istream& operator>>(std::istream& in, CartesianCoordinate<T>& c);


//! Type of the coordinate for a continuous pixel location
class RealPixLoc: public CartesianCoordinate<Real>
{
	public:
		//! Constructor
		RealPixLoc(const Real& x = 0, const Real& y = 0): CartesianCoordinate<Real>(x, y){}
		
		//! Casting from PixLoc
		RealPixLoc(const CartesianCoordinate<unsigned>& c) : CartesianCoordinate<Real>(c.x, c.y){}
		
		//! Casting from CartesianCoordinate
		explicit RealPixLoc(const CartesianCoordinate<Real>& c): CartesianCoordinate<Real>(c){}
		
		//! The null coordinate
		static const RealPixLoc null()
		{
			return RealPixLoc(CartesianCoordinate<Real>::null());
		}
		
		//std::ostream& operator<<(std::ostream& out, const RealPixLoc& c);
		//std::istream& operator>>(std::istream& in, RealPixLoc& c);
};



//! Type of the coordinate for a discrete pixel location
class PixLoc: public CartesianCoordinate<unsigned>
{
	public:
		//! Constructor
		PixLoc(const unsigned& x = 0, const unsigned &y = 0): CartesianCoordinate<unsigned>(x, y){}
		
		//! Casting from RealPixLoc
		explicit PixLoc(const RealPixLoc& c) : CartesianCoordinate<unsigned>(unsigned(c.x+0.5), unsigned(c.y+0.5)){}
		
		//! Casting from CartesianCoordinate
		explicit PixLoc(const CartesianCoordinate<unsigned>& c): CartesianCoordinate<unsigned>(c){}
		
		//! The null coordinate
		static const PixLoc null()
		{
			return PixLoc(CartesianCoordinate<unsigned>::null());
		}
};

//! Type of the HelioProjective cartesian coordinate, in arcsec
class HPC: public CartesianCoordinate<Real>
{
	public:
		//! Constructor
		HPC(const Real& x = 0, const Real& y = 0): CartesianCoordinate<Real>(x, y){}
		
		//! Casting from CartesianCoordinate
		explicit HPC(const CartesianCoordinate<Real>& c): CartesianCoordinate<Real>(c){}
		
		//! The null coordinate
		static const HPC null()
		{
			return HPC(CartesianCoordinate<Real>::null());
		}
};

//! Type of the HelioCentric cartesian coordinate, in Mmeters
class HCC: public CartesianCoordinate<Real>
{
	public:
		Real z;
		
		//! Constructor
		HCC(const Real& x = 0, const Real& y = 0, const Real& z = 0): CartesianCoordinate<Real>(x, y), z(z){}
		
		//! Casting from CartesianCoordinate
		explicit HCC(const CartesianCoordinate<Real>& c): CartesianCoordinate<Real>(c){}
		
		//! The null coordinate
		static const HCC null()
		{
			return HCC(CartesianCoordinate<Real>::null());
		}
};

//! Output operator
std::ostream& operator<<(std::ostream& out, const HCC& c);

//! Input operator
std::istream& operator>>(std::istream& in, HCC& c);

//! General Type for heliographic coordinate
class HeliographicCoordinate
{
	public :
		Real longitude, latitude;

	public :
		//! Constructor
		HeliographicCoordinate():longitude(0), latitude(0){};
		//! Constructor
		HeliographicCoordinate(const Real& longitude, const Real& latitude):longitude(longitude), latitude(latitude){};
		
		//! Destructor
		virtual ~HeliographicCoordinate(){};
		/*
		//! Comparison operator
		bool operator==(const HeliographicCoordinate& c)const
		{
			return longitude == c.longitude && latitude == c.latitude;
		}
		//! Comparison operator
		bool operator!=(const HeliographicCoordinate& c)const
		{
			return longitude != c.longitude || latitude != c.latitude;
		}
		//! Comparison operator
		bool operator<(const HeliographicCoordinate& c)const
		{
			if(latitude < c.latitude)
				return true;
			else if (latitude == c.latitude && longitude < c.longitude)
				return true;
			else
				return false;
		}
		HeliographicCoordinate& operator+=(const HeliographicCoordinate& c)
		{
			longitude += c.longitude;
			latitude += c.latitude;
			return *this;
		}
		HeliographicCoordinate& operator-=(const HeliographicCoordinate& c)
		{
			longitude -= c.longitude;
			latitude -= c.latitude;
			return *this;
		}
		HeliographicCoordinate operator+(const HeliographicCoordinate& c) const
		{
			HeliographicCoordinate result(*this);
			return result+=(c);
		}
		HeliographicCoordinate operator-(const HeliographicCoordinate& c) const
		{
			HeliographicCoordinate result(*this);
			return result-=(c);
		}
		*/
		//! Test if coordinate is null
		virtual bool operator !() const
		{
			return isinf(latitude) || isinf(longitude);
		}
		
	public :
		//! The null coordinate
		static const HeliographicCoordinate null()
		{
			return HeliographicCoordinate(std::numeric_limits<Real>::infinity(), std::numeric_limits<Real>::infinity());
		}
};


//! Distance between 2 coordinates using the Formula of Spherical Law of Cosines
Real distance(const HeliographicCoordinate& a, const HeliographicCoordinate& b, const Real& R = 1);

//! Square of the distance between 2 coordinates using the Formula of Spherical Law of Cosines
Real distance_squared(const HeliographicCoordinate& a, const HeliographicCoordinate& b, const Real& R = 1);

//! Output operator
std::ostream& operator<<(std::ostream& out, const HeliographicCoordinate& c);

//! Input operator
std::istream& operator>>(std::istream& in, HeliographicCoordinate& c);


//! Type of the Heliographic Stonyhurst coordinate, in arcsec
class HGS: public HeliographicCoordinate
{
	public:
		//! Constructor
		HGS(const Real& longitude = 0, const Real& latitude = 0): HeliographicCoordinate(longitude, latitude){}
		
		//! Casting from HeliographicCoordinate
		explicit HGS(const HeliographicCoordinate& c): HeliographicCoordinate(c){}
		
		//! The null coordinate
		static const HGS null()
		{
			return HGS(HeliographicCoordinate::null());
		}
};


//! Type of the Heliographic Carrington coordinate, in arcsec

class HGC: public HeliographicCoordinate
{
	public:
		//! Constructor
		HGC(const Real& longitude = 0, const Real& latitude = 0): HeliographicCoordinate(longitude, latitude){}

		//! Casting from HeliographicCoordinate
		explicit HGC(const HeliographicCoordinate& c): HeliographicCoordinate(c){}

		//! The null coordinate
		static const HGC null()
		{
			return HGC(HeliographicCoordinate::null());
		}

};


#endif
