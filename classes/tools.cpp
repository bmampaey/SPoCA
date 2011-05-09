#include "tools.h"

using namespace std;

inline string itos(const int& i, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << i;
	else
		ss << i;
	return ss.str();
}

inline string dtos(const double& d, const int size)
{
	ostringstream ss;
	if (size > 0)
		ss << setw( size ) << setfill( '0' ) << d;
	else
		ss << d;
	return ss.str();
}



inline int stoi(const string& s)
{
	int i = 0;
	istringstream ss(s);
	ss >> i;
	return i;
}

inline double stod(const string& s)
{
	double d = 0;
	istringstream ss(s);
	ss >> d;
	return d;
}

inline string stripPath(const string &name) 
{
	size_t pos = name.rfind('/');
	return  pos != string::npos ? name.substr(pos+1) : name;
}
inline string stripSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	return  pos != string::npos ? name.substr(0,pos) : name;
}

inline string getPath(const string &name) 
{
	size_t pos = name.rfind('/');
	if (pos == string::npos)
		return "./";
	else
		return  name.substr(0, pos+1);
}

inline string getSuffix(const string &name) 
{
	size_t pos = name.rfind('.');
	if (pos == string::npos)
		return "";
	else
		return  name.substr(pos);
}

inline bool isDir(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) != -1) && (S_ISDIR(statbuf.st_mode));
}

inline bool isFile(const string path)
{
	struct stat statbuf;
	return (stat(path.c_str(), &statbuf) != -1) && (S_ISREG(statbuf.st_mode));
}



inline string time2string(const time_t time)
{
	char datetime_string[100];
	strftime(datetime_string, 100, "%Y%m%d_%H%M%S", gmtime(&time));
	return string(datetime_string);
}

