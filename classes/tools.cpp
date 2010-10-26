#include "tools.h"

using namespace std;



ostream& operator<<(ostream& out, const vector<Real>& v)
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


istream& operator>>(istream& in, vector<Real>& v)
{
	v.resize(0);
	char separator;
	Real value;
	in>>value;
	while (in.good())
	{
		v.push_back(value);
		in>>separator>>value;
	}
	v.push_back(value);
	return in;

}

ostream& operator<<(ostream& out, const vector<unsigned>& v)
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


istream& operator>>(istream& in, vector<unsigned>& v)
{
	v.resize(0);
	char separator;
	unsigned value;
	in>>value;
	while (in.good())
	{
		v.push_back(value);
		in>>separator>>value;
	}
	v.push_back(value);
	return in;

}


string itos(const int& i)
{
	ostringstream ss;
	ss << i;
	return ss.str();
}

string dtos(const double& i)
{
	ostringstream ss;
	ss << i;
	return ss.str();
}

string stripPath(const string &name) 
{
	size_t pos = name.rfind('/');
	return  pos != string::npos ? name.substr(pos+1) : name;
}
string stripSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	return  pos != string::npos ? name.substr(0,pos) : name;
}


