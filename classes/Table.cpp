#include "Table.h"

using namespace std;

Table::~Table()
{}

Table::Table()
{}

Table::Table(const Table& i)
{
	keywords = i.keywords;
}


Table::Table(const Table* i)
{
	keywords = i->keywords;
}

//Accessors
template<class T>
T Table::get(const string& key) const
{
	T value;
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		cerr<<"Warning : No such key in keywords "<<key<<endl;
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<class T>
T Table::get(const char* key) const
{
	T value;
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		cerr<<"Warning : No such key in keywords "<<key<<endl;
	}
	else
	{
		istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<>
bool Table::get<bool>(const string& key) const
{
	return keywords.find(key) != keywords.end();
}

template<>
bool Table::get<bool>(const char* key) const
{
	return  keywords.find(key) != keywords.end();
}

template<>
string Table::get<string>(const string& key) const
{
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		return "";
	}
	return it->second;
}

template<>
string Table::get<string>(const char* key) const
{
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		return "";
	}
	return it->second;
}

template<class T>
void Table::set(const string& key, const T& value)
{
	ostringstream ss;
	ss << value;
	keywords[key] = ss.str();
}

template<class T>
void Table::set(const char* key, const T& value)
{
	ostringstream ss;
	ss << value;
	keywords[key] = ss.str();
}

template<>
void Table::set<string>(const string& key, const string& value)
{
	keywords[key] = value;
}

template<>
void Table::set<string>(const char* key, const string& value)
{
	keywords[key] = value;
}

template<class T>
void Table::set(const string& key, const T& value, const std::string& comment)
{
	set<T>(key,value);
}

template<class T>
void Table::set(const char* key, const T& value, const char* comment)
{
	set<T>(key,value);
}

template<>
void Table::set<string>(const string& key, const string& value, const std::string& comment)
{
	set<string>(key,value);
}

template<>
void Table::set<string>(const char* key, const string& value, const char* comment)
{
	set<string>(key,value);
}

template int Table::get<int>(const std::string& key)const;
template int Table::get<int>(const char* key)const;
template void Table::set<int>(const std::string& key, const int& value);
template void Table::set<int>(const char* key, const int& value);
template unsigned Table::get<unsigned>(const std::string& key)const;
template unsigned Table::get<unsigned>(const char* key)const;
template void Table::set<unsigned>(const std::string& key, const unsigned& value);
template void Table::set<unsigned>(const char* key, const unsigned& value);
template float Table::get<float>(const std::string& key)const;
template float Table::get<float>(const char* key)const;
template void Table::set<float>(const std::string& key, const float& value);
template void Table::set<float>(const char* key, const float& value);
template double Table::get<double>(const std::string& key)const;
template double Table::get<double>(const char* key)const;
template void Table::set<double>(const std::string& key, const double& value);
template void Table::set<double>(const char* key, const double& value);
template string Table::get<string>(const std::string& key)const;
template string Table::get<string>(const char* key)const;
template void Table::set<string>(const std::string& key, const string& value);
template void Table::set<string>(const char* key, const string& value);

template void set<int>(const std::string& key, const int& value, const std::string& comment);
template void set<int>(const char* key, const int& value, const char* comment);
template void set<unsigned>(const std::string& key, const unsigned& value, const std::string& comment);
template void set<unsigned>(const char* key, const unsigned& value, const char* comment);
template void set<float>(const std::string& key, const float& value, const std::string& comment);
template void set<float>(const char* key, const float& value, const char* comment);
template void set<double>(const std::string& key, const double& value, const std::string& comment);
template void set<double>(const char* key, const double& value, const char* comment);
template void set<string>(const std::string& key, const string& value, const std::string& comment);
template void set<string>(const char* key, const string& value, const char* comment);

