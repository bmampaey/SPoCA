#include "Header.h"
#include "tools.h"

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

string Header::expand(const string& text)
{
	string result = text;
	size_t key_start = result.find_first_of('{');
	while (key_start != string::npos)
	{
		size_t key_end = result.find_first_of('}', key_start);
		if(key_end != string::npos)
		{
			string key_name = result.substr(key_start + 1, key_end - key_start - 1);
			string value = "";
			if(has(key_name))
			{
				value = get<string>(key_name);
			}
			else
			{
				cerr<<"Warning: key_name "<<key_name<<" requested in "<<text<<" not found in header."<<endl;
			}
			result.replace(key_start, key_end - key_start + 1, value);
		}
		else
		{
			cerr<<"Warning: malformed string, no closing } after position "<<key_start<<" in "<<text<<endl;
			break;
		}
		key_start = result.find_first_of('{');
	}
	result = replaceAll(result, "\\n", "\n");
	#if defined VERBOSE
	cout<<endl<<text<<" has been expanded to: "<<result<<endl;
	#endif
	return result;
}

