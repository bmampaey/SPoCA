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
	public :
		
		//Constructors and destructors
		Header();
		Header(const Header& i);
		Header(const Header* i);
		~Header();
		
		//Accessors
		template<class T>
		T get(const std::string& key) const;
		template<class T>
		T get(const char* key) const;

		template<class T>
		void set(const std::string& key, const T& value);
		template<class T>
		void set(const char* key, const T& value);
		
		template<class T>
		void set(const std::string& key, const T& value, const std::string& comment);
		template<class T>
		void set(const char* key, const T& value, const char* comment);
		
		// Iterator
		typedef std::map<std::string,std::string>::iterator iterator;
		typedef std::map<std::string,std::string>::const_iterator const_iterator;
		iterator begin() { return keywords.begin(); }
		iterator end() { return keywords.end(); }
		const_iterator begin() const { return keywords.begin(); } 
		const_iterator end() const { return keywords.end(); }

};




#endif
