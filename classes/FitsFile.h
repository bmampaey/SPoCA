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
#include "Coordinate.h"


//! Class that manage i/o to a fits file
/*!
The class serve to hide all the complexity of the cfitsio library,and implements some common read and write operations that we use across the progams. 

*/

class FitsFile
{

	protected :
		//! The filename of the FitsFile
		std::string filename;
		//! Internal cfitsio structure
		fitsfile  *fptr;
		//! The status of the FitsFile
		/*! A status of 0 means ok, otherwise there is an error */
		int status;
	
	protected :
		//! Routine to convert a C++ data type to a cfitsio datatype 
		int fitsDataType(const std::type_info& t);
		//! Routine to get the best bitpix value for a cfitsio datatype 
		int getBitpix(int datatype);
		//! Routine to get the best column format value for a cfitsio datatype
		std::string getFormat(int datatype);
		//! Routine to get the Current HDU type
		int get_CHDU_type();
		
	public :
		//! Constructor
		FitsFile();
		//! Constructor, open the specified fits file
		/*! @param mode The mode specifies how to open the file.
				Possible values are FitsFile::overwrite or FitsFile::update
				If not specified, and the file exist it will be opened read only.
		*/
		FitsFile(const std::string& filename, const int mode = 0);
		//! Destructor
		~FitsFile();
		
		//! Routine to close the fits file
		void close();
		//! Routine to test the status of the fits file
		bool isClosed();
		//! Routine to test the status of the fits file
		bool isGood();
		
		//! Routine to move to another HDU
		FitsFile& moveTo(int extension_number);
		//! Routine to move to another HDU
		FitsFile& moveTo(const std::string& extension_name);
		//! Routine to test if an HDU exist
		bool has(const std::string& extension_name);
		
		//! Routine to read a Fits header
		FitsFile& readHeader(Header& header);
		//! Routine to write a Fits header
		FitsFile& writeHeader(const Header& header);
		
		//! Routine to read a 2D image
		//! @tparam T Type of the pixels
		template<class T>
		FitsFile& readImage(T*& image, unsigned &X, unsigned& Y, T* null = NULL);
		//! Routine to write a 2D image
		//! @tparam T Type of the pixels
		/*! @param mode The mode specifies how to write the image.
				Possible values are FitsFile::update and/or FitsFile::compress (they can be specified together with a | )
				By default a new image will be appended to the fits file.
		*/
		template<class T>
		FitsFile& writeImage(T* image, const unsigned X, const unsigned Y, int mode = 0, const std::string name = "");
		
		//! Routine to write a binary table
		FitsFile& writeTable(const std::string &name, const unsigned number_rows = 0);
		//! Routine to write a column in the current table
		//! @tparam T Type of the column data
		/*! @param mode The mode specifies how to write the column.
				Possible value are FitsFile::overwrite 
				By default the data will be appended to the column if it exists already.
		*/
		template<class T>
		FitsFile& writeColumn(const std::string &name, const std::vector<T>& data, const int mode = 0);
		//! Routine to read a column in the current table
		//! @tparam T Type of the column data
		template<class T>
		FitsFile& readColumn(const std::string &name, std::vector<T>& data);
	
		//! Routine to find a column by it's name in a table
		int find_column(const std::string &name);
	
	public:
		//! Constants for mode options
		static const int overwrite = 1;
		//! Constants for mode options
		static const int update = 2;
		//! Constants for mode options
		static const int compress = 4;
};

//! Routine that convert a string date (iso/fits formed) to a time_t
time_t iso2ctime(const std::string& date);

#endif
