#include "Coordinate.h"

using namespace std;

const Coordinate Coordinate::Max(std::numeric_limits<unsigned>::max(), std::numeric_limits<unsigned>::max());

ostream& operator<<(ostream& out, const Coordinate& c)
{
	out<<"("<<c.x<<","<<c.y<<")";
	return out;
}


istream& operator>>(istream& in, Coordinate& c)
{
	char separator;
	in>>separator>>c.x>>separator>>c.y>>separator;
	return in;
}
