#pragma once
#ifndef FitsFile_H
#define FitsFile_H

#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <vector>
#include <ctime>

#include "fitsio.h"
#include "longnam.h"
#include "tools.h"
#include "constants.h"
#include "Header.h"




class FitsFile
{

	protected :
		std::string filename;
		fitsfile  *fptr;
		int status;
	
	protected :
		//Routines for the automatic conversion between C type and Fits types
		int fitsDataType(const std::type_info& t);
		int getBitpix(int datatype);
		std::string getFormat(int datatype);
		int find_column(const std::string &name);
		int get_CHDU_type();
		
	public :
		//Constructors and destructors
		FitsFile();
		FitsFile(const std::string& filename, const int mode = 0);
		~FitsFile();
		
		//Routines to close the fits file
		void close();
		//Routines to test the fits file
		bool isClosed();
		bool isGood();
		
		//Routines to move into the fits file
		FitsFile& moveTo(int extension_number);
		FitsFile& moveTo(const std::string& extension_name);
		bool has(const std::string& extension_name);
		
		//Routines to read and write Fits header
		FitsFile& readHeader(Header& header);
		FitsFile& writeHeader(const Header& header);
		
		//Routines to read and write Fits 2D image
		template<class T>
		FitsFile& readImage(T*& image, unsigned &X, unsigned& Y, T* nullvalue = NULL);
		template<class T>
		FitsFile& writeImage(T* image, const unsigned X, const unsigned Y, int mode = 0);
		
		//Routines to read and write Fits binary table and columns
		FitsFile& writeTable(const std::string &name, const unsigned number_rows = 0);
		template<class T>
		FitsFile& writeColumn(const std::string &name, const std::vector<T>& data, const int mode = 0);
		template<class T>
		FitsFile& readColumn(const std::string &name, std::vector<T>& data);
	
	public:
		// Constants for mode options
		static const int overwrite = 1;
		static const int update = 2;
		static const int compress = 4;
};

// Routine that convert a string date (iso/fits formed) to a time_t
time_t iso2ctime(const std::string& date);

#endif
