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
	if(! in.good())
	{
		cerr<<"Error parsing Vector from stream"<<endl;
		return in;
	}
	if(in.peek() == '[')
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



inline string itos(const int& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
		ss << i;
	return ss.str();
}

inline string dtos(const double& d, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << d;
	else
		ss << d;
	return ss.str();
}



inline int stoi(const string& s)
{
	int i = 0;
	istringstream ss(s);
	ss >> i;
	return i;
}

inline double stod(const string& s)
{
	double d = 0;
	istringstream ss(s);
	ss >> d;
	return d;
}

inline string stripPath(const string &name) 
{
	size_t pos = name.rfind('/');
	return  pos != string::npos ? name.substr(pos+1) : name;
}
inline string stripSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	return  pos != string::npos ? name.substr(0,pos) : name;
}

inline string getPath(const string &name) 
{
	size_t pos = name.rfind('/');
	if (pos == string::npos)
		return "./";
	else
		return  name.substr(0, pos+1);
}

inline string getSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	if (pos == string::npos)
		return "";
	else
		return  name.substr(pos);
}

inline bool isDir(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) != -1) && (S_ISDIR(statbuf.st_mode));
}

inline bool isFile(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) != -1) && (S_ISREG(statbuf.st_mode));
}



inline string time2string(const time_t time)
{
	char datetime_string[100];
	strftime(datetime_string, 100, "%Y%m%d_%H%M%S", gmtime(&time));
	return string(datetime_string);
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

