#include "FitsHeader.h"

using namespace std;

FitsHeader::~FitsHeader()
{}

FitsHeader::FitsHeader(const string& filename)
{
	fitsfile  *fptr;
	int   status  = 0;
	
	if (fits_open_image(&fptr, filename.c_str(), READONLY, &status))
	{
		cerr<<"Error : opening file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return;
	} 
	bool readStatus = readHeader(fptr);
	if ( fits_close_file(fptr, &status) )
	{
		cerr<<"Error : closing file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	} 
}

FitsHeader::FitsHeader(const FitsHeader& i)
{
	header = i.header;
}


FitsHeader::FitsHeader(const FitsHeader* i)
{
	header = i->header;
}


bool FitsHeader::readHeader(fitsfile* fptr)
{
	int   status  = 0;
	char key[81];
	char value[81];
	char* comment = NULL;					  /**<By specifying NULL we say that we don't want the comments	*/


	//We first need to reset the fptr to the beginning
	if( fits_read_record (fptr, 0, key, &status))
	{
		cerr<<"Error reseting the fits pointer to the beginning of the header for file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = KEY_NO_EXIST;
	}
	int i = 0;
	while(status != KEY_NO_EXIST)
	{
		status = 0;
		if(fits_read_keyn(fptr, ++i, key, value, comment, &status))
		{
			if(status != KEY_NO_EXIST)
			{
				cerr<<"Error reading keyword from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = KEY_NO_EXIST;
			}
		}
		else
		{
			header[key]=value;
		}
	} 

	return status == KEY_NO_EXIST;
}


bool FitsHeader::writeHeader(fitsfile* fptr)
{
	int   status  = 0;
	char* comment = NULL;
	
	for ( map<string,string>::iterator i = header.begin(); i != header.end(); ++i )
	{
		if(fits_update_key(fptr, TSTRING, i->first.c_str(), const_cast<char *>(i->second.c_str()), comment, &status))
		{
			cerr<<"Error : writing keyword to file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			status = 0;
		} 
	}
	return (status == 0);
}



//Accessors
template<class T>
T FitsHeader::get(const string& key)
{
	T value;
	if(header[key].empty())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
	}
	else
	{
		istringstream ss(header[key]);
		ss >> value;
	}
	return value;
}

template<class T>
T FitsHeader::get(const char* key)
{
	T value;
	if(header[key].empty())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
	}
	else
	{
		istringstream ss(header[key]);
		ss >> value;
	}
	return value;
}

template<class T>
void FitsHeader::set(const string& key, const T& value)
{
	ostringstream ss;
	ss << value;
	header[key] = ss.str();
}

template<class T>
void FitsHeader::set(const char* key, const T& value)
{
	ostringstream ss;
	ss << value;
	header[key] = ss.str();
}

template int FitsHeader::get(const std::string& key);
template int FitsHeader::get(const char* key);
template void FitsHeader::set(const std::string& key, const int& value);
template void FitsHeader::set(const char* key, const int& value);
template unsigned FitsHeader::get(const std::string& key);
template unsigned FitsHeader::get(const char* key);
template void FitsHeader::set(const std::string& key, const unsigned& value);
template void FitsHeader::set(const char* key, const unsigned& value);
template float FitsHeader::get(const std::string& key);
template float FitsHeader::get(const char* key);
template void FitsHeader::set(const std::string& key, const float& value);
template void FitsHeader::set(const char* key, const float& value);
template double FitsHeader::get(const std::string& key);
template double FitsHeader::get(const char* key);
template void FitsHeader::set(const std::string& key, const double& value);
template void FitsHeader::set(const char* key, const double& value);
template string FitsHeader::get(const std::string& key);
template string FitsHeader::get(const char* key);
template void FitsHeader::set(const std::string& key, const string& value);
template void FitsHeader::set(const char* key, const string& value);

