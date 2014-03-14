#include "Header.h"

using namespace std;

Header::~Header()
{
	#if defined VERBOSE
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



bool Header::has(const string& key) const
{
	return keywords.find(key) != keywords.end();
}


bool Header::has(const char* key) const
{
	return  keywords.find(key) != keywords.end();
}

template<>
string Header::get<string>(const string& key) const
{
	map<string,string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if defined VERBOSE
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
		return "";
	}
	return it->second;
}

string Header::comment(const string& key) const
{
	map<string,string>::const_iterator i = keywords_comments.find(key);
	if(i != keywords_comments.end())
		return i->second;
	else
		return "";
}

template<>
void Header::set<string>(const string& key, const string& value, const string& comment)
{
	keywords[key] = value;
	if(!comment.empty())
		keywords_comments[key] = comment;
}

