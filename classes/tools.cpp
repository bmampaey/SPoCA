#include "tools.h"

using namespace std;



ostream& operator<<(ostream& out, const vector<Real>& v)
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
			out<<" "<<v.at(i);

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


string itos(const int& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
		ss << i;
	return ss.str();
}

string dtos(const double& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
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
	return  pos != string::npos ? name.substr(0,pos-1) : name;
}

string getPath(const string &name) 
{
	size_t pos = name.rfind('/');
	if (pos == string::npos)
		return "./";
	else
		return  name.substr(0, pos+1);
}

string getSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	if (pos == string::npos)
		return "";
	else
		return  name.substr(pos);
}

bool isDir(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) != -1) && (S_ISDIR(statbuf.st_mode));
}

string time2string(const time_t time)
{
	char datetime_string[100];
	strftime(datetime_string, 100, "%Y%m%d_%H%M%S", gmtime(&time));
	return string(datetime_string);
}

