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


std::ostream& operator<<(std::ostream& out, const std::vector<Real>& v);
std::istream& operator>>(std::istream& in, std::vector<Real>& v);

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned>& v);
std::istream& operator>>(std::istream& in, std::vector<unsigned>& v);

std::string itos(const int& i, const int size = 0);
std::string dtos(const double& i, const int size = 0);

std::string stripPath(const std::string &name); 
std::string stripSuffix(const std::string &name); 

bool isDir(const std::string path);
std::string time2string(const time_t time);

#endif
