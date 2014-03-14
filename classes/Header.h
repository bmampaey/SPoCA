#pragma once
#ifndef Header_H
#define Header_H

#include <sstream>
#include <iostream>
#include <typeinfo>
#include <string>
#include <map>
#include <ctime>
#include <cstring>


class Header
{
	private:
		std::map<std::string,std::string> keywords;
		std::map<std::string,std::string> keywords_comments;
	public :
		
		//! Constructor
		Header();
		//! Copy Constructor
		Header(const Header& i);
		//! Copy Constructor
		Header(const Header* i);
		//! Destructor
		~Header();
		
		//! Ckeck if the keyword key is in the header
		bool has(const std::string& key) const;
		//! Ckeck if the keyword key is in the header
		bool has(const char* key) const;
		
		//! Return the value of the keyword key
		template<class T>
		T get(const std::string& key) const;
		
		//! Get comment for keyword key
		std::string comment(const std::string& key) const;
		
		//! Set/Update the value and the comment of the keyword key
		template<class T>
		void set(const std::string& key, const T& value, const std::string& comment = "");
		
		//! Iterator
		typedef std::map<std::string,std::string>::iterator iterator;
		//! Const Iterator
		typedef std::map<std::string,std::string>::const_iterator const_iterator;
		iterator begin() { return keywords.begin(); }
		iterator end() { return keywords.end(); }
		const_iterator begin() const { return keywords.begin(); } 
		const_iterator end() const { return keywords.end(); }

};

template<class T>
T Header::get(const std::string& key) const
{
	T value = 0;
	std::map<std::string,std::string>::const_iterator it = keywords.find(key);
	if(it == keywords.end())
	{
		#if defined VERBOSE
		cerr<<"Warning : No such key in keywords "<<key<<endl;
		#endif
	}
	else
	{
		std::istringstream ss(it->second);
		ss >> value;
	}
	return value;
}

template<>
std::string Header::get(const std::string& key) const;

template<class T>
void Header::set(const std::string& key, const T& value, const std::string& comment)
{
	std::ostringstream ss;
	ss << value;
	keywords[key] = ss.str();
	if(!comment.empty())
		keywords_comments[key] = comment;
}

template<>
void Header::set(const std::string& key, const std::string& value, const std::string& comment);


#endif
