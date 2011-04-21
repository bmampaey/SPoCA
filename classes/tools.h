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

std::ostream& operator<<(std::ostream& out, const std::vector<Real>& v);
std::istream& operator>>(std::istream& in, std::vector<Real>& v);

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned>& v);
std::istream& operator>>(std::istream& in, std::vector<unsigned>& v);

//! Convert a integer as a string
/*! @param size Minimal size of the integer. It will be padded on the left with 0 if needed */
std::string itos(const int& i, const int size = 0);
//! Convert a double as a string
/*! @param size Minimal size of the double. It will be padded on the left with 0 if needed */
std::string dtos(const double& i, const int size = 0);

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
//! Return a time as a string of the form yyyymmdd_hhmmss
std::string time2string(const time_t time);

#endif
