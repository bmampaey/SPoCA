#pragma once
#ifndef Tools_H
#define Tools_H

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "constants.h"


std::ostream& operator<<(std::ostream& out, const std::vector<Real>& v);
std::istream& operator>>(std::istream& in, std::vector<Real>& v);

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned>& v);
std::istream& operator>>(std::istream& in, std::vector<unsigned>& v);

std::string itos(const int& i);
std::string dtos(const double& i);

std::string stripPath(const std::string &name); 
std::string stripSuffix(const std::string &name); 

#endif
