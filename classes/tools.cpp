
#include "tools.h"

using namespace std;

//! String vector input
template<>
istream& operator>>(istream& in, vector<string>& v)
{
	v.clear();
	while(in.good() && isspace(char(in.peek())))
	{
		in.get();
	}
	if(in.eof())
	{
		return in;
	}
	else if(!in)
	{
		throw std::runtime_error("Error parsing vector from stream");
		return in;
	}
	else if(in.peek() == '[')
	{
		in.get();
		char c;
		in.get(c);
		string value;
		while (in.good())
		{
			if(c == ',')
			{
				v.push_back(value);
				value = "";
			}
			else
			{
				value += c;
			}
			in.get(c);
			if(c == ']')
			{
				v.push_back(value);
				break;
			}
		}
	}
	else
	{
		char c;
		in.get(c);
		string value;
		while (in.good())
		{
			if(c == ',')
			{
				v.push_back(value);
				value = "";
			}
			else
			{
				value += c;
			}
			in.get(c);
			if(! in.good() || isspace(c))
			{
				v.push_back(value);
				break;
			}
		}
	}
	return in;
}

string toString(const time_t time)
{
	char datetime_string[100];
	strftime(datetime_string, 100, "%Y%m%d_%H%M%S", gmtime(&time));
	return string(datetime_string);
}

string toString(const int& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
		ss << i;
	return ss.str();
}

string toString(const unsigned& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
		ss << i;
	return ss.str();
}

string toString(const double& d, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << d;
	else
		ss << d;
	return ss.str();
}



int toInt(const string& s)
{
	int i = 0;
	istringstream ss(s);
	ss >> i;
	return i;
}

unsigned toUnsigned(const string& s)
{
	unsigned i = 0;
	istringstream ss(s);
	ss >> i;
	return i;
}

double toDouble(const string& s)
{
	double d = 0;
	istringstream ss(s);
	ss >> d;
	return d;
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

string makePath(const string &path1, const string &path2)
{
	if(path1.empty())
		return path2;
	else if(path2.empty())
		return path1;
	#if defined _WIN32
	if(*path1.rbegin() == '\\')
		return path1 + path2;
	else
		return path1 + "\\" + path2;
	#else
	if(*path1.rbegin() == '/')
		return path1 + path2;
	else
		return path1 + "/" + path2;
	#endif
}


bool isDir(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) == 0) && (S_ISDIR(statbuf.st_mode));
}

bool isFile(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) == 0) && (S_ISREG(statbuf.st_mode));
}

bool emptyFile(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) == 0) && (statbuf.st_size == 0);
}

vector<string> split(const string &s, const char delim)
{
	vector<string> elems;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

string replaceAll(const string& str, const string& from, const string& to)
{
	if(from.empty())
		return str;
	string result = str;
	size_t start_pos = 0;
	while((start_pos = result.find(from, start_pos)) != string::npos)
	{
		result.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return result;
}

string trimWhites(const string& str)
{
	string result = str;
	size_t pos = result.find_first_not_of(" \t");
	if(pos != string::npos)
		result.erase(0, pos);
	else
		result.clear();
	return result;
}

#define ELEM_SWAP(a,b) { register EUVPixelType t=(a);(a)=(b);(b)=t; }
/* !
This Quickselect routine is based on the algorithm described in
"Numerical recipes in C", Second Edition,
Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
This code by Nicolas Devillard - 1998. Public domain.
 */
EUVPixelType quickselect(deque<EUVPixelType>& arr, Real percentil)
{

	int low = 0 ;
	int high = arr.size()-1 ;
	int median = low + int((high - low) * percentil);
	
	int middle, ll, hh;
	
	for (;;)
	{
		if (high <= low)						  /* One element only */
			return arr[median] ;

		if (high == low + 1)					  /* Two elements only */
		{
			if (arr[low] > arr[high])
				ELEM_SWAP(arr[low], arr[high]) ;
			return arr[median] ;
		}

		/* Find median of low, middle and high items; swap into position low */
		middle = low + int((high - low) * percentil);
		if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
		if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
		if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

		/* Swap low item (now in position middle) into position (low+1) */
		ELEM_SWAP(arr[middle], arr[low+1]) ;

		/* Nibble from each end towards middle, swapping items when stuck */
		ll = low + 1;
		hh = high;
		for (;;)
		{
			do ll++; while (arr[low] > arr[ll]) ;
			do hh--; while (arr[hh]  > arr[low]) ;

			if (hh < ll)
				break;

			ELEM_SWAP(arr[ll], arr[hh]) ;
		}

		/* Swap middle item (in position low) back into correct position */
		ELEM_SWAP(arr[low], arr[hh]) ;

		/* Re-set active partition */
		if (hh <= median)
			low = ll;
		if (hh >= median)
			high = hh - 1;
	}
}
#undef ELEM_SWAP
