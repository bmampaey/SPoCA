#pragma once
#ifndef FitsHeader_H
#define FitsHeader_H

#include <sstream>
#include <iostream>
#include <typeinfo>
#include <string>
#include <map>
#include <ctime>

#include "fitsio.h"
#include "longnam.h"

class FitsHeader
{

	protected :
		
		std::map<std::string,std::string> header;
		
	public :
		
		//Constructors and destructors
		FitsHeader(const std::string& filename);
		FitsHeader(const FitsHeader& i);
		FitsHeader(const FitsHeader* i);
		~FitsHeader();
		
		//Routines to read and write a fits header
          bool writeHeader(fitsfile* fptr);
          bool readHeader(fitsfile* fptr);
		
		//Accessors
		template<class T>
		T get(const std::string& key);
		template<class T>
		T get(const char* key);
		template<class T>
		void set(const std::string& key, const T& value);
		template<class T>
		void set(const char* key, const T& value);
		
};




#endif
