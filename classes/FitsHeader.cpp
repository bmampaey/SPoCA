#include "FitsHeader.h"

using namespace std;

FitsHeader::~FitsHeader()
{}

FitsHeader::FitsHeader()
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
	readKeywords(fptr);
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


bool FitsHeader::readKeywords(fitsfile* fptr)
{
	int   status  = 0;
	char key[81];
	char value[81];
	char* comment = NULL;					  /**<By specifying NULL we say that we don't want the comments	*/

	const char* exclist[] = {"SIMPLE", "BITPIX", "NAXIS", "EXTEND", "Z", "XTENSION", "TTYPE1", "TFORM1", "PCOUNT", "GCOUNT", "TFIELDS", "END"};

	//We first need to reset the fptr to the beginning
	if( fits_read_record (fptr, 0, key, &status))
	{
		cerr<<"Error reseting the fits pointer to the beginning of the header for file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = 0;
	}


	int i = 0;
	while(status != KEY_OUT_BOUNDS)
	{
		status = 0;
		if(fits_read_keyn(fptr, ++i, key, value, comment, &status))
		{
			if(status != KEY_OUT_BOUNDS)
			{
				cerr<<"Error reading keyword from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = 0;
			}
		}
		else
		{
			// We need to verify the keyword must not be excluded
			bool excluded = false;
			for(unsigned i = 0; i < sizeof(exclist) / sizeof(const char*) && !excluded; ++i)
			{
				excluded = strncmp(exclist[i], key, strlen(exclist[i])) == 0;
			}	
			if(!excluded)
			{
				if(value[0] == '\'')
				{
					*(strrchr(value, '\'')) = '\0';
					header[key]=value+1;
				}
				else
				{
					header[key]=value;
				}
			}
		}
	} 
	#if DEBUG >= 3
	cout<<"Header for file "<<fptr->Fptr->filename<<endl; 
	for ( map<string,string>::iterator i = header.begin(); i != header.end(); ++i )
	{
		cout<<i->first<<":\t"<<i->second<<endl;
	}
	#endif
	return status == KEY_OUT_BOUNDS;
}


bool FitsHeader::writeKeywords(fitsfile* fptr)
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
T FitsHeader::get(const string& key) const
{
	T value;
	map<string,string>::const_iterator it = header.find(key);
	if(it == header.end())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<class T>
T FitsHeader::get(const char* key) const
{
	T value;
	map<string,string>::const_iterator it = header.find(key);
	if(it == header.end())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<>
bool FitsHeader::get<bool>(const string& key) const
{
	return header.find(key) != header.end();
}

template<>
bool FitsHeader::get<bool>(const char* key) const
{
	return  header.find(key) != header.end();
}

template<>
string FitsHeader::get<string>(const string& key) const
{
	map<string,string>::const_iterator it = header.find(key);
	if(it == header.end())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
		return "";
	}
	return it->second;
}

template<>
string FitsHeader::get<string>(const char* key) const
{
	map<string,string>::const_iterator it = header.find(key);
	if(it == header.end())
	{
		cerr<<"Warning : No such key in header "<<key<<endl;
		return "";
	}
	return it->second;
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

template<>
void FitsHeader::set<string>(const string& key, const string& value)
{
	header[key] = value;
}

template<>
void FitsHeader::set<string>(const char* key, const string& value)
{
	header[key] = value;
}

template int FitsHeader::get<int>(const std::string& key)const;
template int FitsHeader::get<int>(const char* key)const;
template void FitsHeader::set<int>(const std::string& key, const int& value);
template void FitsHeader::set<int>(const char* key, const int& value);
template unsigned FitsHeader::get<unsigned>(const std::string& key)const;
template unsigned FitsHeader::get<unsigned>(const char* key)const;
template void FitsHeader::set<unsigned>(const std::string& key, const unsigned& value);
template void FitsHeader::set<unsigned>(const char* key, const unsigned& value);
template float FitsHeader::get<float>(const std::string& key)const;
template float FitsHeader::get<float>(const char* key)const;
template void FitsHeader::set<float>(const std::string& key, const float& value);
template void FitsHeader::set<float>(const char* key, const float& value);
template double FitsHeader::get<double>(const std::string& key)const;
template double FitsHeader::get<double>(const char* key)const;
template void FitsHeader::set<double>(const std::string& key, const double& value);
template void FitsHeader::set<double>(const char* key, const double& value);
template string FitsHeader::get<string>(const std::string& key)const;
template string FitsHeader::get<string>(const char* key)const;
template void FitsHeader::set<string>(const std::string& key, const string& value);
template void FitsHeader::set<string>(const char* key, const string& value);

