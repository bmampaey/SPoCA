#include "Header.h"

using namespace std;

Header::~Header()
{
	#if DEBUG >= 3
		cerr<<"Destructor for Header called"<<endl;
	#endif
}

Header::Header()
{}

Header::Header(const Header& i)
{
	keywords = i.keywords;
}


Header::Header(const Header* i)
{
	keywords = i->keywords;
}

//Accessors
template<class T>
T Header::get(const string& key) const
{
	T value = 0;
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if DEBUG>= 1
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<class T>
T Header::get(const char* key) const
{
	T value = 0;
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if DEBUG>= 1
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<>
string Header::get<string>(const string& key) const
{
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if DEBUG>= 1
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
		return "";
	}
	return it->second;
}


template<>
string Header::get<string>(const char* key) const
{
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if DEBUG>= 1
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
		return "";
	}
	return it->second;
}


bool Header::has(const string& key) const
{
	return keywords.find(key) != keywords.end();
}


bool Header::has(const char* key) const
{
	return  keywords.find(key) != keywords.end();
}


template<class T>
void Header::set(const string& key, const T& value)
{
	ostringstream ss;
	ss << value;
	keywords[key] = ss.str();
}

template<class T>
void Header::set(const char* key, const T& value)
{
	ostringstream ss;
	ss << value;
	keywords[key] = ss.str();
}

template<>
void Header::set<string>(const string& key, const string& value)
{
	keywords[key] = value;
}

template<>
void Header::set<string>(const char* key, const string& value)
{
	keywords[key] = value;
}


template<class T>
void Header::set(const string& key, const T& value, const string& comment)
{
	set<T>(key,value);
}

template<class T>
void Header::set(const char* key, const T& value, const char* comment)
{
	set<T>(key,value);
}

template<>
void Header::set<string>(const string& key, const string& value, const string& comment)
{
	set<string>(key,value);
}

template<>
void Header::set<string>(const char* key, const string& value, const char* comment)
{
	set<string>(key,value);
}



template int Header::get<int>(const string& key)const;
template int Header::get<int>(const char* key)const;
template void Header::set<int>(const string& key, const int& value);
template void Header::set<int>(const char* key, const int& value);
template unsigned Header::get<unsigned>(const string& key)const;
template unsigned Header::get<unsigned>(const char* key)const;
template void Header::set<unsigned>(const string& key, const unsigned& value);
template void Header::set<unsigned>(const char* key, const unsigned& value);
template unsigned short Header::get<unsigned short>(const string& key)const;
template unsigned short Header::get<unsigned short>(const char* key)const;
template void Header::set<unsigned short>(const string& key, const unsigned short& value);
template void Header::set<unsigned short>(const char* key, const unsigned short& value);
template float Header::get<float>(const string& key)const;
template float Header::get<float>(const char* key)const;
template void Header::set<float>(const string& key, const float& value);
template void Header::set<float>(const char* key, const float& value);
template double Header::get<double>(const string& key)const;
template double Header::get<double>(const char* key)const;
template void Header::set<double>(const string& key, const double& value);
template void Header::set<double>(const char* key, const double& value);
template bool Header::get<bool>(const string& key)const;
template bool Header::get<bool>(const char* key)const;
template void Header::set<bool>(const string& key, const bool& value);
template void Header::set<bool>(const char* key, const bool& value);

template void Header::set<int>(const string& key, const int& value, const string& comment);
template void Header::set<int>(const char* key, const int& value, const char* comment);
template void Header::set<unsigned>(const string& key, const unsigned& value, const string& comment);
template void Header::set<unsigned>(const char* key, const unsigned& value, const char* comment);
template void Header::set<unsigned short>(const string& key, const unsigned short& value, const string& comment);
template void Header::set<unsigned short>(const char* key, const unsigned short& value, const char* comment);
template void Header::set<float>(const string& key, const float& value, const string& comment);
template void Header::set<float>(const char* key, const float& value, const char* comment);
template void Header::set<double>(const string& key, const double& value, const string& comment);
template void Header::set<double>(const char* key, const double& value, const char* comment);
template void Header::set<bool>(const string& key, const bool& value, const string& comment);
template void Header::set<bool>(const char* key, const bool& value, const char* comment);


