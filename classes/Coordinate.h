#pragma once
#ifndef Coordinate_H
#define Coordinate_H

#include <stdio.h> 
#include <limits>
#include <iostream>

class Coordinate
{
	public :
		unsigned  x, y;

	public :
		//Constructors
		Coordinate():x(0), y(0){};
		Coordinate(const unsigned& x, const unsigned& y):x(x), y(y){};
		Coordinate(const unsigned& v):x(v), y(v){};
		//operators
		bool operator==(const Coordinate& c)const
		{
			return x == c.x && y == c.y;
		}
		bool operator!=(const Coordinate& c)const
		{
			return x != c.x || y != c.y;
		}
		bool operator<(const Coordinate& c)const
		{
			if(y < c.y)
				return true;
			else if (y == c.y && x < c.x)
				return true;
			else
				return false;
		}
		Coordinate& operator+=(const Coordinate& c)
		{
			x += c.x;
			y += c.y;
			return *this;
		}
		Coordinate& operator-=(const Coordinate& c)
		{
			x -= c.x;
			y -= c.y;
			return *this;
		}
		Coordinate operator+(const Coordinate& c) const
		{
			Coordinate result(*this);
			return result+=(c);
		}
		Coordinate operator-(const Coordinate& c) const
		{
			Coordinate result(*this);
			return result-=(c);
		}
		double d2(const Coordinate& c) const
		{
			return (x - c.x) * (x - c.x) + (y - c.y) * (y - c.y);
		}

	public :
		static const Coordinate Max;
		friend std::ostream& operator<<(std::ostream& out, const Coordinate& c);
		friend std::istream& operator>>(std::istream& in, Coordinate& c);

};


#endif
