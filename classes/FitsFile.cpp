#include "FitsFile.h"

using namespace std;

FitsFile::FitsFile()
:fptr(NULL),status(0)
{
}

FitsFile::FitsFile(const string& filename, const int mode)
:filename(filename),fptr(NULL),status(0)
{
	int exists;
	if(fits_file_exists(filename.c_str(), &exists, &status))
	{
		cerr<<"Error : testing file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		exists = 0;
	}
	bool overwrite_ = mode & overwrite;
	if(overwrite_ || exists <= 0)
	{
		remove(filename.c_str());
		if (fits_create_file(&fptr, filename.c_str(), &status))
		{
			cerr<<"Error : opening file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			
			fptr = NULL;
		}

	}
	else
	{
		int iomode = mode & update ? READWRITE : READONLY;
		if (fits_open_image(&fptr, filename.c_str(), iomode, &status))
		{
			cerr<<"Error : opening file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			fptr = NULL;
		}
	}

}

FitsFile::~FitsFile()
{
	if(! isClosed())
		close();
	#if DEBUG >= 3
		cerr<<"Destructor for FitsFile called (filename = "<<filename<<")"<<endl;
	#endif
}

void FitsFile::close()
{
	status = 0;
	if ( fits_close_file(fptr, &status) )
	{
		cerr<<"Error : closing file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
	} 
	fptr = NULL;
	status = 0;
}

bool FitsFile::isClosed()
{
	return fptr == NULL;
}
bool FitsFile::isGood()
{
	return status == 0;
}

FitsFile& FitsFile::moveTo(int extension_number)
{
	int *hdutype = NULL;
	if (fits_movabs_hdu(fptr, extension_number, hdutype, &status) )
	{
		cerr<<"Error : moving to extension "<<extension_number<<" in file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
	} 
	return *this;
}

FitsFile& FitsFile::moveTo(const string& extension_name)
{
	int hdutype = ANY_HDU;
	char* extname = const_cast<char *>(extension_name.c_str());
	int extver = 0;
	if (fits_movnam_hdu(fptr, hdutype, extname, extver, &status) )
	{
		cerr<<"Error : moving to extension "<<extension_name<<" in file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
	} 
	return *this;

}

bool FitsFile::has(const string& extension_name)
{
	// We save the current_hdu
	int current_hdu;
	fits_get_hdu_num(fptr, &current_hdu);
	// We check if we can move to the researched hdu
	char* extname = const_cast<char *>(extension_name.c_str());
	int temp_status = 0;
	fits_movnam_hdu(fptr, ANY_HDU, extname, 0, &temp_status);
	// We move back to the current hdu
	moveTo(current_hdu);

	return temp_status == 0;

}

//Routine to determine the fits code for a C datatype 
int FitsFile::fitsDataType(const type_info& t)
{
	int datatype;
	if(t == typeid(double))
		datatype = TDOUBLE;
	else if(t == typeid(float))
		datatype = TFLOAT;
	else if(t == typeid(long))
		datatype = TLONG;
	else if(t == typeid(unsigned long))
		//datatype = TULONG;
		datatype = TLONG;
	else if(t == typeid(short))
		datatype = TSHORT;
	else if(t == typeid(unsigned short))
		//datatype = TUSHORT;
		datatype = TSHORT;
	else if(t == typeid(int))
		datatype = TINT;
	else if(t == typeid(unsigned int))
		//datatype = TUINT;
		datatype = TINT;
	else if(t == typeid(char))
		datatype = TBYTE;
	else if(t == typeid(signed char))
		//datatype = TSBYTE;
		datatype = TBYTE;
	else if(t == typeid(char*))
		datatype = TSTRING;
	else
	{
		cerr<<"Error: unknown Fits data type for type "<<t.name()<<endl;
		datatype = 0;
		
	}
	return datatype;
}

string FitsFile::getFormat(int datatype)
{
	string format;
	switch(datatype)
	{
		case TDOUBLE:
			format = "D";
			break;
		case TFLOAT:
			format = "E";
			break;
		case TLONG:
			format = "K";
			break;
		case TULONG:
			format = "K";//There is no 64 bit unsigned
			break;
		case TINT:
			format = "J";
			break;
		case TUINT:
			#ifdef STRICTFITS
			format = "J";
			#else
			format = "V";
			#endif
			break;
		case TSHORT:
			format = "I";
			break;
		case TUSHORT:
			#ifdef STRICTFITS
			format = "I";
			#else
			format = "U";
			#endif
			break;
		case TBYTE:
			format = "B";
			break;
		case TSBYTE:
			#ifdef STRICTFITS
			format = "B";
			#else
			format = "S";
			#endif
			break;
		case TSTRING:
			format = "A";
			break;
		default:
			cerr<<"Error determining format for datatype "<<datatype<<". Setting it to string."<<endl;
			format = "A";
	}
	return format;
}


int FitsFile::getBitpix(int datatype)
{
	int bitpix;
	switch(datatype)
	{
		case TDOUBLE:
			bitpix = DOUBLE_IMG;
			break;
		case TFLOAT:
			bitpix = FLOAT_IMG;
			break;
		case TLONG:
			bitpix = LONG_IMG;
			break;
		case TULONG:
			bitpix = LONG_IMG;
			break;
		case TINT:
			bitpix = LONG_IMG;
			break;
		case TUINT:
			bitpix = LONG_IMG;
			break;
		case TSHORT:
			bitpix = SHORT_IMG;
			break;
		case TUSHORT:
			bitpix = SHORT_IMG;
			break;
		case TBYTE:
			bitpix = BYTE_IMG;
			break;
		case TSBYTE:
			bitpix = BYTE_IMG;
			break;
		default:
			cerr<<"Error determining bitpix for datatype "<<datatype<<". Setting it to double."<<endl;
			bitpix = DOUBLE_IMG;
	}
	return bitpix;
}


FitsFile& FitsFile::readHeader(Header& header)
{
	const int exclude_keywords_class[] = {TYP_STRUC_KEY, TYP_CMPRS_KEY, TYP_SCAL_KEY, TYP_HDUID_KEY,TYP_CKSUM_KEY, TYP_COMM_KEY};
	const unsigned ekc_size = sizeof(exclude_keywords_class) / sizeof(exclude_keywords_class[0]);

	if(isClosed())
	{
		cerr<<"Error reading header, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	
	
	//We first need to reset the fptr to the beginning
	char card[FLEN_CARD];
	if( fits_read_record (fptr, 0, card, &status))
	{
		cerr<<"Error reseting the fits pointer to the beginning of the header for file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = 0;
	}

	char value[FLEN_VALUE];
	char key[FLEN_KEYWORD];
	int keylength = 0;
	char* comment = NULL; //By specifying NULL we say that we don't want the comments
	int i = 0;
	while(status != KEY_OUT_BOUNDS)
	{
		status = 0;
		if(fits_read_record(fptr, ++i, card, &status))
		{
			if(status != KEY_OUT_BOUNDS)
			{
				cerr<<"Error reading keyword from file "<<filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			}
		}
		else
		{
			// We need to verify the keyword must not be excluded
			
			bool excluded = false;
			int keyword_class = fits_get_keyclass(card);
			for(unsigned i = 0; i < ekc_size && !excluded; ++i)
			{
				excluded = (keyword_class == exclude_keywords_class[i]);
			}
			if(!excluded)
			{
				if(fits_get_keyname(card, key, &keylength, &status))
				{
					cerr<<"Error parsing keyword from file "<<filename<<" card : "<< card <<" : "<< status <<endl;
					fits_report_error(stderr, status);
					status = 0;
					continue;
				}
				if(fits_parse_value(card, value, comment, &status))
				{
					cerr<<"Error parsing value from file "<<filename<<" card : "<< card <<" : "<< status <<endl;
					fits_report_error(stderr, status);
					status = 0;
					continue;
				}
				//We need to cleanup the value for leading/trailing spaces and quotes
				string cleanValue(value);
				size_t startpos = cleanValue.find_first_not_of("' \t");
				size_t endpos = cleanValue.find_last_not_of("' \t");
				if((startpos != string::npos) && (endpos != string::npos))
				{
					header.set(key,cleanValue.substr(startpos, endpos-startpos+1));
				}
			}
		}
	}
	if(status == KEY_OUT_BOUNDS)
		status = 0;
	#if DEBUG >= 3
	cout<<"Header for file "<<filename<<endl; 
	for ( map<string,string>::iterator i = header.begin(); i != header.end(); ++i )
	{
		cout<<setw(8)<<left<<i->first<<": "<<i->second<<endl;
	}
	#endif
	return *this;
}

FitsFile& FitsFile::writeHeader(const Header& header)
{
	if(isClosed())
	{
		cerr<<"Error writing header, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	char* comment = NULL;
	for ( Header::const_iterator i = header.begin(); i != header.end(); ++i )
	{
		// To avoid cfitsio to put quotes around values (why does it do it in the first place ???) we have to know if the value is a number or not
		// We cannot use fits_get_keytype to determine the type of the value because it is bugged
		if(i->second.find_first_not_of(" \t") == string::npos)
		{
			// The value is all white
			if(fits_update_key_null(fptr, i->first.c_str(), comment, &status))
			{
				cerr<<"Error : writing keyword "<<i->first<<" to file "<<filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			}
		}
		else if(i->second.find_first_not_of(" \t0123456789.-") != string::npos)
		{
			// It is a string
			if(fits_update_key(fptr, TSTRING, i->first.c_str(), const_cast<char *>(i->second.c_str()), comment, &status))
			{
				cerr<<"Error : writing keyword "<<i->first<<" to file "<<filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			} 
		}
		else if(i->second.find_first_of(".") == string::npos)
		{
			//It is probably a integer
			int value = stoi(i->second);
			if(fits_update_key(fptr, TINT, i->first.c_str(), &value, comment, &status))
			{
				cerr<<"Error : writing keyword "<<i->first<<" to file "<<filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			} 
		
		}
		else
		{
			//It is probably a double
			double value = stod(i->second);
			if(fits_update_key(fptr, TDOUBLE, i->first.c_str(), &value, comment, &status))
			{
				cerr<<"Error : writing keyword "<<i->first<<" to file "<<filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			} 
		
		}
	}
	return *this;
}

int FitsFile::get_CHDU_type()
{
	int hdutype = ANY_HDU;
	if (fits_get_hdu_type(fptr, &hdutype, &status) )
	{
		cerr<<"Error : could not get current HDU type :"<< status <<endl;			
		fits_report_error(stderr, status);
	}
	return hdutype;
}

template<class T>
FitsFile& FitsFile::readImage(T*& image, unsigned &X, unsigned& Y, T* null)
{
	if (isClosed())
	{
		cerr<<"Error reading image, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	//We determine the datatype 
	int datatype = fitsDataType(typeid(T));
	if (datatype == 0 || datatype == TSTRING)
	{
		cerr<<"Error reading image from file "<<filename<<" : Unknown type "<< typeid(T).name() <<endl;
		
		return *this;
	}

	// We read the image parameters
	int bitpix, naxis;
	long axes[2];
	if (fits_get_img_param(fptr, 2, &bitpix, &naxis, axes, &status))
	{
		cerr<<"Error : reading image parameters from file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
		return *this;
	}
	
	if(naxis != 2)
	{
		cerr<<"Error : image is not 2D "<<endl;			
		
		return *this;
	}
	X = axes[0];
	Y = axes[1];
	unsigned numberPixels = X * Y;
	
	// We alocate space for the pixels
	#if DEBUG >= 1
	if (image != NULL)
	{
		cerr<<"Error : should not allocate memory to a non NULL pointer when reading image from file "<<filename<<endl;			
	}
	#endif
	image = new T[numberPixels];
	
	// We read the pixels
	int anynull;
	if (fits_read_img(fptr, datatype, 1, numberPixels, null, image, &anynull, &status))
	{
		cerr<<"Error : reading image from file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
		return *this;
	}
	
	#if DEBUG >= 1
	// We check that there is no lost of precision

	switch(bitpix)
	{
		case BYTE_IMG:
			//Can't put a unsigned into a signed of same size
			if(datatype != TBYTE)
				break;
		case USHORT_IMG:
			//Can't put a unsigned into a signed of same size or smaller
			if(datatype != TBYTE && datatype != TSHORT)
				break;
		case ULONG_IMG:
			//Can't put a unsigned into a signed of same size or smaller
			if(datatype != TBYTE && datatype != TSHORT && datatype != TLONG && datatype != TINT)
				break;
		case SBYTE_IMG:
		case SHORT_IMG:
			if(datatype == TSHORT)
				break;
		case LONG_IMG:
			if(datatype == TLONG || datatype == TINT)
				break;
		case LONGLONG_IMG:
			if(datatype == TLONGLONG)
				break;
		case FLOAT_IMG:
			if(datatype == TFLOAT)
				break;
		case DOUBLE_IMG:
			if(datatype != TDOUBLE)
			{
				cerr<<"Warning : Fits File image data type and Image data type mismatch, you may lose precision or sign."<<endl;
			}
			break;
		default:
			cerr<<"Error : Unknown fits data type."<<endl;
	}
	
	#endif
	return *this;

}



template<class T>
FitsFile& FitsFile::writeImage(T* image, const unsigned X, const unsigned Y, int mode, const string name)
{
	if (isClosed())
	{
		cerr<<"Error writing image, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	//We determine the datatype 
	int datatype = fitsDataType(typeid(T));
	if (datatype == 0 || datatype == TSTRING)
	{
		cerr<<"Error writing image from file "<<filename<<" : Unknown type "<< typeid(T).name() <<endl;
		
		return *this;
	}
	
	//We determine bitpix so we write with the correct type
	int bitpix = getBitpix(datatype);

	// We get the axes from the image
	long axes[2];
	axes[0] = X;
	axes[1] = Y;
	
	int naxis = 2;

	/*if(mode & overwrite)
	{
		int hdutype = ANY_HDU;
		if (fits_get_hdu_type(fptr, &hdutype, &status) )
		{
			cerr<<"Error : could not get current HDU type :"<< status <<endl;			
			fits_report_error(stderr, status);
		}
		if(hdutype!= IMAGE_HDU)
		{
			cerr<<"Error : current HDU is not an image, cannot overwrite."<<endl;			
			fits_report_error(stderr, status);
			return *this;
		}
		if(fits_delete_hdu(fptr, NULL, &status) )
		{
			cerr<<"Error : could not overwrite image :"<< status <<endl;			
			fits_report_error(stderr, status);
		}
		else if(fits_movrel_hdu(fptr, -1, NULL, &status))
		{
			#if DEBUG >= 1
			cerr<<"Error : moving to previous hdu :"<< status <<endl;			
			fits_report_error(stderr, status);
			#endif
		}
		if(mode & compress)
		{
			if ( fits_set_compression_type(fptr, RICE_1, &status) )
			{
				cerr<<"Error : could not set image compression :"<< status <<endl;			
				fits_report_error(stderr, status);
				status = 0;
			}
		}
		if (fits_insert_img(fptr, bitpix, naxis, axes, &status))
		{
			cerr<<"Error : creating image in file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			return *this;
		} 
	}*/
	if(mode & update)
	{
		if(get_CHDU_type() != IMAGE_HDU)
		{
			cerr<<"Error : current HDU is not an image, cannot update."<<endl;			
			fits_report_error(stderr, status);
			return *this;
		}
		// We read the current image parameters to make sure they match
		int cbitpix, cnaxis;
		long caxes[2];
		if(fits_get_img_param(fptr, 2, &cbitpix, &cnaxis, caxes, &status))
		{
			cerr<<"Error : reading image parameters from file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			return *this;
		}
		if(cbitpix != bitpix || cnaxis != naxis || caxes[0] != axes[0] || caxes[1] != axes[1])
		{
			#if DEBUG >= 1
			cerr<<"Warning : possible image parameter mismatch while updating image in file "<<filename<<endl;
			#endif
		}
	}
	else
	{
		if(mode & compress)
		{
			if ( fits_set_compression_type(fptr, RICE_1, &status) )
			{
				cerr<<"Error : could not set image compression :"<< status <<endl;			
				fits_report_error(stderr, status);
				status = 0;
			}
		}
		if(fits_create_img(fptr, bitpix, naxis, axes, &status))
		{
			cerr<<"Error : creating image in file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			return *this;
		} 
	}


	unsigned numberPixels = X*Y;
	if ( fits_write_img(fptr, datatype, 1, numberPixels, image, &status) )
	{
		cerr<<"Error : writing pixels to file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
		return *this;
	} 
	if(! name.empty())
	{
		if(fits_update_key(fptr, TSTRING, "EXTNAME", const_cast<char *>(name.c_str()), NULL, &status))
		{
			cerr<<"Error : setting image name to file "<<filename<<" :"<< status <<endl;
			fits_report_error(stderr, status);
			status = 0;
		} 
	}
	return *this;

}


FitsFile& FitsFile::writeTable(const string &name, const unsigned number_rows)
{
	if (isClosed())
	{
		cerr<<"Error writing column, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	if (fits_create_tbl(fptr, BINARY_TBL, number_rows, 0, NULL, NULL, NULL, const_cast<char *>(name.c_str()), &status))
	{
		cerr<<"Error : writing table to file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
	}
	return *this;
}

int FitsFile::find_column(const string &name)
{
	int colnum = 0;
	char* tplate = const_cast<char *>(name.c_str());
	char colname[FLEN_VALUE]; //A column name is the value of a keyword
	int casesen = CASEINSEN;
	if (fits_get_colname(fptr, casesen, tplate, colname, &colnum, &status))
	{
		if(status == COL_NOT_UNIQUE || (status =! COL_NOT_FOUND && name != colname))
		{
			#if DEBUG >= 1
			cerr<<"Warning : searching column "<<name<<". Column name is not unique or exact, using column with name "<<colname<<"."<<endl;
			#endif
			status = 0;
		}
		else
		{
			return 0;
		}
	}
	return colnum;
}

template<class T>
FitsFile& FitsFile::writeColumn(const string &name, const vector<T>& data, const int mode)
{
	if (isClosed())
	{
		cerr<<"Error writing column, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	int datatype = fitsDataType(typeid(T));
	if (datatype == 0 || datatype == TSTRING)
	{
		cerr<<"Error writing column to file "<<filename<<" : Unknown type "<< typeid(T).name() <<endl;
		return *this;
	}
	
	// We search for the column number
	int colnum = 0;
	if(mode & overwrite)
	{
		colnum = find_column(name);
		// If we overwrite and the column exist, we erase it
		if(colnum != 0)
		{
			if (fits_delete_col(fptr, colnum, &status) )
			{
				cerr<<"Error : delting column from file "<<filename<<" :"<< status <<endl;			
				fits_report_error(stderr, status);
			}
		}
	}
	if(colnum == 0)
	{
		if (fits_get_num_cols(fptr, &colnum, &status) )
		{
			cerr<<"Error : writing column from file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
		}
		++colnum;
	}
	
	// We create the column
	char* ttype = const_cast<char *>(name.c_str());
	string format = getFormat(datatype);
	char* tform = const_cast<char *>(format.c_str());
	if (fits_insert_col(fptr, colnum, ttype, tform, &status) )
	{
		cerr<<"Error : writing column to file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
		return *this;
	}
	// We add the data to the column
	if(data.size() == 0)
		return *this;
	int firstrow = 1;
	int firstelem = 1;
	int nelements = data.size();
	void * array = (void*) &(data[0]);

	if (fits_write_col(fptr, datatype, colnum, firstrow, firstelem, nelements, array, &status) )
	{
		cerr<<"Error : writing value to column "<<name<<" in file "<<filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		
	}
	return *this;
}

template<class T>
FitsFile& FitsFile::readColumn(const string &name, vector<T>& data)
{
	if (isClosed())
	{
		cerr<<"Error reading column, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	int datatype = fitsDataType(typeid(T));
	if (datatype == 0 || datatype == TSTRING)
	{
		cerr<<"Error reading column to file "<<filename<<" : Unknown type "<< typeid(T).name() <<endl;
		return *this;
	}
	
	// We search for the column number
	int colnum = find_column(name);
	if (colnum == 0)
	{
		cerr<<"Error : searching column "<<name<<" in file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return *this;
	}
	
	// We get the number of rows and adjust the size of data accordingly
	long nrows = 0;
	if(fits_get_num_rows(fptr, &nrows, &status))
	{
		cerr<<"Error : reading number of rows in table of file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return *this;
	}
	data.resize(nrows);
	
	if(nrows == 0)
		return *this;
	
	// We read the column data
	void *nulval = NULL;
	void * array = (void *) &(data[0]);
	int anynul;
	if (fits_read_col(fptr, datatype, colnum, 1, 1, nrows, nulval, array, &anynul, &status))
	{
		cerr<<"Error : reading values from column "<<name<<" in file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		
		return *this;
	}
	return *this;
}


template FitsFile& FitsFile::writeImage(ColorType* image, const unsigned X, const unsigned Y, int mode, const string name);
template FitsFile& FitsFile::readImage(ColorType*& image, unsigned &X, unsigned& Y, ColorType* null);

template FitsFile& FitsFile::writeImage(EUVPixelType* image, const unsigned X, const unsigned Y, int mode, const string name);
template FitsFile& FitsFile::readImage(EUVPixelType*& image, unsigned &X, unsigned& Y, EUVPixelType* null);

template FitsFile& FitsFile::writeColumn(const string &name, const vector<int>& array, const int mode);
template FitsFile& FitsFile::writeColumn(const string &name, const vector<unsigned>& array, const int mode);
template FitsFile& FitsFile::writeColumn(const string &name, const vector<unsigned short>& array, const int mode);
template FitsFile& FitsFile::writeColumn(const string &name, const vector<float>& array, const int mode);
template FitsFile& FitsFile::writeColumn(const string &name, const vector<double>& array, const int mode);
template FitsFile& FitsFile::writeColumn(const string &name, const vector<char>& array, const int mode);


template FitsFile& FitsFile::readColumn(const string &name,  vector<int>& array);
template FitsFile& FitsFile::readColumn(const string &name,  vector<unsigned>& array);
template FitsFile& FitsFile::readColumn(const string &name,  vector<unsigned short>& array);
template FitsFile& FitsFile::readColumn(const string &name,  vector<float>& array);
template FitsFile& FitsFile::readColumn(const string &name,  vector<double>& array);
template FitsFile& FitsFile::readColumn(const string &name,  vector<char>& array);

template<>
FitsFile& FitsFile::writeColumn(const string &name, const vector<string>& data, const int mode)
{
	if (isClosed())
	{
		cerr<<"Error writing column, "<<filename<<" is closed"<<endl;
		
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		
		return *this;
	}
	
	// We search for the column number
	int colnum = 0;
	if(mode & overwrite)
	{
		colnum = find_column(name);
		// If we overwrite and the column exist, we erase it
		if(colnum != 0)
		{
			if (fits_delete_col(fptr, colnum, &status) )
			{
				cerr<<"Error : delting column from file "<<filename<<" :"<< status <<endl;			
				fits_report_error(stderr, status);
			}
		}
	}
	if(colnum == 0)
	{
		if (fits_get_num_cols(fptr, &colnum, &status) )
		{
			cerr<<"Error : writing column from file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
		}
		++colnum;
	}
	
	// We need the max width of the strings for the format
	unsigned stringwidth = 0;
	for (unsigned s = 0; s < data.size(); ++s)
		stringwidth = data[s].length() > stringwidth ?  data[s].length() : stringwidth;
		
	string format = itos(stringwidth)+ "A";
	char* tform = const_cast<char *>(format.c_str());
	
	// We create the column

	char* ttype = const_cast<char *>(name.c_str());
	if (fits_insert_col(fptr, colnum, ttype, tform, &status))
	{
		cerr<<"Error : writing column to file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	
		return *this;
	}

	if(data.size() == 0)
		return *this;

	// We add the data to the column
	int nelements = data.size();
	char** values = new char*[nelements];
	for (unsigned s = 0; s < data.size(); ++s)
		values[s] = const_cast<char *>(data[s].c_str());

	if (fits_write_col_str(fptr, colnum, 1, 1, nelements, values, &status) )
	{
		cerr<<"Error : writing values to column "<<name<<" in file "<<filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		
	}
	
	delete []values;
	return *this;
}

template<>
FitsFile& FitsFile::readColumn(const string &name, vector<string>& data)
{
	if (isClosed())
	{
		cerr<<"Error reading column, "<<filename<<" is closed"<<endl;
		return *this;
	}
	if(!isGood())
	{
		cerr<<"Error "<<filename<<" is not good"<<endl;
		return *this;
	}
	
	// We search for the column number
	int colnum = find_column(name);
	if (colnum == 0)
	{
		return *this;
	}
	
	// We get the number of rows and adjust the size of data accordingly
	long nrows = 0;
	if(fits_get_num_rows(fptr, &nrows, &status))
	{
		cerr<<"Error : reading number of rows in table of file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return *this;
	}
	data.resize(nrows);
	
	if(nrows == 0)
		return *this;
	
	// We get the max size of strings
	int maxsringsize = 1000;
	if(fits_get_col_display_width(fptr, colnum, &maxsringsize, &status))
	{
		cerr<<"Error : determinig max string size of column "<<name<<" in file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	}
	
	// We read the column data
	char nulstr[] = "";
	char *value = new char[maxsringsize + 1];
	int anynul;
	for (long firstrow = 1; firstrow <= nrows && status == 0 ; ++firstrow)
	{
		if (fits_read_col_str (fptr, colnum, firstrow, 1, 1, nulstr, &value, &anynul, &status))
		{
			cerr<<"Error : reading value from column "<<name<<" in file "<<filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
		}
		else
			data[firstrow - 1] = value;
	}
	delete []value;
	return *this;
}

//! Write a list of PixLoc coordinates to a FitsFile, adding 1 to conform to fits standard 
template<>
FitsFile& FitsFile::writeColumn(const string &name, const vector<PixLoc>& data, const int mode)
{
	string xname = 'X' + name;
	string yname = 'Y' + name;
	
	vector<unsigned> xarray(data.size());
	vector<unsigned> yarray(data.size());
	for (unsigned d = 0; d < data.size(); ++d)
	{
		xarray[d] = data[d].x + 1;
		yarray[d] = data[d].y + 1;
	}
	writeColumn(xname, xarray, mode);
	writeColumn(yname, yarray, mode);
	return *this;
}

//! Read a list of PixLoc coordinates from a FitsFile, substracting 1 to conform to fits standard 
template<>
FitsFile& FitsFile::readColumn(const string &name, vector<PixLoc>& data)
{
	string xname = 'X' + name;
	string yname = 'Y' + name;
	
	vector<unsigned> xarray;
	vector<unsigned> yarray;
	readColumn(xname, xarray);
	readColumn(yname, yarray);
	if (xarray.size() != yarray.size())
	{
		cerr<<"Error reading column "<<name<<", number of x coordinate is different than y coordinate!"<<endl;
		data.clear();
	}
	else
	{
		data.resize(xarray.size());
		for (unsigned d = 0; d < data.size(); ++d)
		{
			data[d].x = xarray[d] - 1;
			data[d].y = yarray[d] - 1;
		}
	}
	return *this;
}

//! Write a list of RealPixLoc coordinates to a FitsFile, adding 1 to conform to fits standard 
template<>
FitsFile& FitsFile::writeColumn(const string &name, const vector<RealPixLoc>& data, const int mode)
{
	string xname = 'X' + name;
	string yname = 'Y' + name;
	
	vector<Real> xarray(data.size());
	vector<Real> yarray(data.size());
	for (unsigned d = 0; d < data.size(); ++d)
	{
		xarray[d] = data[d].x + 1;
		yarray[d] = data[d].y + 1;
	}
	writeColumn(xname, xarray, mode);
	writeColumn(yname, yarray, mode);
	return *this;
}

//! Read a list of RealPixLoc coordinates from a FitsFile, substracting 1 to conform to fits standard 
template<>
FitsFile& FitsFile::readColumn(const string &name, vector<RealPixLoc>& data)
{
	string xname = 'X' + name;
	string yname = 'Y' + name;
	
	vector<Real> xarray;
	vector<Real> yarray;
	readColumn(xname, xarray);
	readColumn(yname, yarray);
	if (xarray.size() != yarray.size())
	{
		cerr<<"Error reading column "<<name<<", number of x coordinate is different than y coordinate!"<<endl;
		data.clear();
	}
	else
	{
		data.resize(xarray.size());
		for (unsigned d = 0; d < data.size(); ++d)
		{
			data[d].x = xarray[d] - 1;
			data[d].y = yarray[d] - 1;
		}
	}
	return *this;
}

time_t iso2ctime(const string& date)
{
	int status = 0;
	tm time;
	double seconds;
	if (fits_str2time(const_cast<char *>(date.c_str()), &(time.tm_year), &(time.tm_mon), &(time.tm_mday), &(time.tm_hour), &(time.tm_min), &seconds, &status))
	{
		cerr<<"Error converting date_obs to time : "<< status <<endl;
		fits_report_error(stderr, status);
	}
	else
	{
		time.tm_sec = int(seconds);
		time.tm_mon -= 1;	//Because stupid c++ standard lib has the month going from 0-11
		time.tm_year -= 1900;	//Because stupid c++ standard lib thinks the world started in 1900
		time.tm_isdst = 0;
	}
	return timegm(&time);
}

