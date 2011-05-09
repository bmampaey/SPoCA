#pragma once
#ifndef Tools_H
#define Tools_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include <sys/stat.h>
#include "constants.h"

/*!
@file tools.h
A collection of little function to make life easier.

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
	in>>trash>>value;
	while (in.good())
	{
		v.push_back(value);
		in>>trash>>value;
	}
	in>>trash;
	v.push_back(value);
	return in;

}

//! Convert a integer as a string
/*! @param size Minimal size of the integer. It will be padded on the left with 0 if needed */
std::string itos(const int& i, const int size = 0);
//! Convert a double as a string
/*! @param size Minimal size of the double. It will be padded on the left with 0 if needed */
std::string dtos(const double& d, const int size = 0);

//! Convert a string to a integer
int stoi(const std::string& s);
//! Convert a string to a double
double stod(const std::string& s);

//! Return the name of the file without the preceding path (/ si the directory limiter)
std::string stripPath(const std::string &name); 
//! Return the path of the file without the last suffix (. is the suffix delimitor)
std::string stripSuffix(const std::string &name);
//! Return the path of the directory containing the file (/ si the directory limiter)
std::string getPath(const std::string &name);
//! Return the last suffix of the file (. is the suffix delimitor)
std::string getSuffix(const std::string &name);
//! Check if the path is an existing directory
bool isDir(const std::string path);
//! Check if the path is an existing and regular file
bool isFile(const std::string path);
//! Return a time as a string of the form yyyymmdd_hhmmss
std::string time2string(const time_t time);

#endif
