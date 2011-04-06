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


ostream& operator<<(ostream& out, const vector<Coordinate>& v)
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

istream& operator>>(istream& in, vector<Coordinate>& v)
{
	v.resize(0);
	char separator;
	Coordinate value;
	in>>value;
	while (in.good())
	{
		v.push_back(value);
		in>>separator>>value;
	}
	v.push_back(value);
	return in;

}
