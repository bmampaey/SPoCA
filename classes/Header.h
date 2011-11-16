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
		
		//! Constructor
		Header();
		//! Copy Constructor
		Header(const Header& i);
		//! Copy Constructor
		Header(const Header* i);
		//! Destructor
		~Header();
		
		//! Return the value of the keyword key
		template<class T>
		T get(const std::string& key) const;
		//! Return the value of the keyword key
		template<class T>
		T get(const char* key) const;
		
		//! Ckeck if the keyword key is in the header
		bool has(const std::string& key) const;
		//! Ckeck if the keyword key is in the header
		bool has(const char* key) const;
		
		//! Set/Update the value of the keyword key
		template<class T>
		void set(const std::string& key, const T& value);
		//! Set/Update the value of the keyword key
		template<class T>
		void set(const char* key, const T& value);
		
		//! Set/Update the value and the comment of the keyword key
		/*! (Not yet implemented) */
		template<class T>
		void set(const std::string& key, const T& value, const std::string& comment);
		//! Set/Update the value and the comment of the keyword key
		/*! (Not yet implemented) */
		template<class T>
		void set(const char* key, const T& value, const char* comment);
		
		//! Iterator
		typedef std::map<std::string,std::string>::iterator iterator;
		//! Const Iterator
		typedef std::map<std::string,std::string>::const_iterator const_iterator;
		iterator begin() { return keywords.begin(); }
		iterator end() { return keywords.end(); }
		const_iterator begin() const { return keywords.begin(); } 
		const_iterator end() const { return keywords.end(); }

};




#endif
