#pragma once
#ifndef Tools_H
#define Tools_H

#include <vector>
#include <deque>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include <sys/stat.h>
#include <locale>
#include <map>
#include <stdexcept> 
#include "constants.h"

/*!
@file tools.h
A collection of little functions to make life easier.
*/

//! General vector output
template<class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
	if (v.size() > 0)
	{
		out<<"["<<v.at(0);
		for (unsigned i = 1 ; i < v.size() ; ++i)
			out<<","<<v.at(i);
		out<<"]";
	}
	else
	{
		out<<"[]";
	}

	return out;
}

//! General vector input
template<class T>
std::istream& operator>>(std::istream& in, std::vector<T>& v)
{
	v.clear();
	char trash;
	T value;
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
		in>>trash>>value;
		while (in.good() && in.peek() != ']')
		{
			v.push_back(value);
			in>>trash>>value;
		}
		in>>trash;
		v.push_back(value);
	}
	else
	{
		in>>value;
		while (in.good() && ! isspace(char(in.peek())))
		{
			v.push_back(value);
			in>>trash>>value;
		}
		v.push_back(value);
	}
	return in;
}

//! String vector input
template<>
std::istream& operator>>(std::istream& in, std::vector<std::string>& v);

//! Convert a vector to a string
template<class T>
std::string toString(const std::vector<T>& v)
{
	std::ostringstream tmp;
	tmp << v;
	return tmp.str();
}

//! Convert a string to a vector
template<class T>
std::vector<T> toVector(const std::string& s)
{
	std::vector<T> tmp;
	std::istringstream tmp2(s);
	tmp2 >> tmp;
	return tmp;
}

//! Convert a deque to a string
template<class T>
std::string toString(const std::deque<T>& v)
{
	return toString(std::vector<T>(v.begin(), v.end()));
}

//! Convert a time to a string of the form yyyymmdd_hhmmss
std::string toString(const time_t time);

//! Convert a integer as a string
/*! @param size Minimal size of the integer. It will be padded on the left with 0 if needed */
std::string toString(const int& i, const int size = 0);

//! Convert an unsigned as a string
/*! @param size Minimal size of the integer. It will be padded on the left with 0 if needed */
std::string toString(const unsigned& i, const int size = 0);

//! Convert a double as a string
/*! @param size Minimal size of the double. It will be padded on the left with 0 if needed */
std::string toString(const double& d, const int size = 0);

//! Convert a string to a unsigned
unsigned toUnsigned(const std::string& s);

//! Convert a string to a integer
int toInt(const std::string& s);

//! Convert a string to a double
double toDouble(const std::string& s);

//! General string parsing
template<class T>
std::string& operator>>(std::string& input, T& value)
{
	std::istringstream in(input);
	in>>value;
	
	input.erase(0,in.tellg());
	return input;
}

//! Return a vector of a map keys
template<class K, class V>
std::vector<K> keys(const std::map<K,V>& M)
{
	std::vector<K> result;
	result.reserve(M.size());
	for(typename std::map<K, V>::const_iterator m = M.begin(); m != M.end(); ++m)
		result.push_back(m->first);
	return result;
}

//! Return a vector of the map values
template<class K, class V>
std::vector<V> values(const std::map<K,V>& M)
{
	std::vector<V> result;
	result.reserve(M.size());
	for(typename std::map<K, V>::const_iterator m = M.begin(); m != M.end(); ++m)
		result.push_back(m->second);
	return result;
}

//! Split a string into substring using delimiter
std::vector<std::string> split(const std::string &s, const char delim = ',');

//! Replace all occurences of from to to in string str
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

//! Remove all white space and tab at the beggining and end of str
std::string trimWhites(const std::string& str);

//! Return the name of the file without the preceding path (/ si the directory limiter)
std::string stripPath(const std::string &name);

//! Return the path of the file without the last suffix (. is the suffix delimitor)
std::string stripSuffix(const std::string &name);

//! Return the path of the directory containing the file (/ si the directory limiter)
std::string getPath(const std::string &name);

//! Return the last suffix of the file (. is the suffix delimitor)
std::string getSuffix(const std::string &name);

//! Return the join path with the correct OS separator
std::string makePath(const std::string &path1, const std::string &path2);

//! Check if the path is an existing directory
bool isDir(const std::string path);

//! Check if the path is an existing and regular file
bool isFile(const std::string path);

//! Check if the path is an empty file
bool emptyFile(const std::string path);

//! Routine that return the requested percentil of an array
/* The array will be modified */ 
EUVPixelType quickselect(std::deque<EUVPixelType>& arr, Real percentil);


#endif
