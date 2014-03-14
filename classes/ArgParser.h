#pragma once
#ifndef ArgParser_H
#define ArgParser_H

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <deque>
#include <typeinfo>

//! General class for parsing arguments & config files
/* ! 

This class allow to define an argument parser that can parse both command line arguments and config files.
Command line arguments always take precedence on the config files options.

It is possible to organise the parameters in sections.

BNF of the accepted command line arguments

arguments ::= arguments argument | arguments "--" values | ""
argument ::= long_param
			| long_param value
			| short_param
			| short_param value
			| section_argument
			| value

long_param ::= "--" parameter_id | "--" section_id "." parameter_id
short_param ::= "-" letter | "-" section_id "." letter
section_argument ::= "--" section_id ":"
values ::= values value | "" 
parameter_id ::= letter (letter|digit|’_’)*
section_id ::= "" | identifier
value ::= (char)*

BNF for the accepted config file options
line ::= comment | section | section comment | option | option comment
comment ::= "#" (char)* | ";" (char)*
section ::= "[" section_id "]" | section_id ":"
option :: parameter_id "=" value | parameter_id value
parameter_id ::= letter (letter|digit|’_’)*
section_id ::= "" | identifier
value ::= char (char)* | "\"" (char)* "\""
*/

class ArgParser
{
	public:
		class Parameter
		{
			private:
				char short_name;
				std::string description;
				std::string default_value;
				std::string value;
				bool set;
				unsigned position;
				bool config_file;
				bool help;
				bool remaining_parameters;
				int remaining_parameter_min;
				int remaining_parameter_max;
				
				friend class ArgParser;
			
			private:
				std::string help_message(const std::string& long_name, std::string section, bool config_file = false, bool doxygen = false);
			
			public:
				Parameter(const char short_name = '\0', const std::string& description = "");
				
				template<class T>
				Parameter(const T& default_value, const char short_name = '\0', const std::string& description = "");
				
				template<class T>
				Parameter(const T& default_value, const std::string& description = "");
				
				template<class T>
				operator T() const;
				
				template<class T>
				bool operator ==(const T& v) const;
				
				bool operator ==(const char* v) const;
				
				bool is_set() const;
				
				bool is_positional() const;
				
				bool is_config_file() const;
				
				bool is_help() const;
				
				bool is_remaining_parameters() const;
				
				friend bool compare_parameter_position(const Parameter* c1, const Parameter* c2 );
				friend std::ostream& operator<<(std::ostream& os, const Parameter& parameter);
		};
	
	public:
		std::string description;
	
	private:
		typedef std::map<std::string, std::map<std::string, ArgParser::Parameter> > SectionList;
		SectionList sections;
		std::vector<Parameter> configuration_files;
		std::deque<std::string> positional_arguments;
		static unsigned positional_counter;
		static const std::string alpha_num_under;
	
	private:
		bool parse_positional_only(std::string& argument);
		bool parse_section(std::string& argument, std::string& section);
		bool parse_section_parameter(std::string& argument, std::string& section);
		bool parse_long_parameter(std::string& argument, std::string& parameter, std::string& section);
		bool parse_option(std::string& argument, std::string& parameter, std::string& section, std::string& value);
		bool parse_short_parameter(std::string& argument, std::string& parameter, std::string& section);
		bool parse_section_prefix(std::string& argument, std::string& section);
		bool parse_parameter_id(std::string& argument, std::string& identifier);
		bool parse_section_id(std::string& argument, std::string& identifier);
		bool parse_value(std::string& argument, std::string& value);
		bool parse_comment(std::string& argument);
		std::string find_long_parameter_name(const char short_name, const std::string section = "general");
	
	public:
		ArgParser(std::string description = "");
		std::map<std::string, ArgParser::Parameter>& operator() (const std::string& index = "general");
		Parameter& operator[] (const std::string& index);
		static Parameter PositionalParameter(const std::string& description = "");
		static Parameter RemainingPositionalParameters(const std::string& description = "", unsigned min = 0, int max = -1);
		static Parameter ConfigurationFile(const std::string& description = "Program option configuration file.", const std::string& default_value = "");
		static Parameter ConfigurationFile(const char short_name, const std::string& description = "Program option configuration file.", const std::string& default_value = "");
		static Parameter Help(const char short_name = '\0');
		void parse(int argc, const char **argv, bool warn_duplicate = true, bool overwrite = true);
		void parse_file(const std::string& filename, bool warn_duplicate = true, bool overwrite = true);
		std::string section_help_message(const std::string& section_name, bool config_file = false, bool doxygen = false);
		std::string section_help_message(const std::string& section_name, std::vector<std::pair<unsigned, std::string> >& positional_parameters, std::string& remaining_parameters_name, bool config_file = false, bool doxygen = false);
		std::string help_message(const std::string& executable = "exec", bool config_file = false, bool doxygen = false);
		std::deque<std::string> RemainingPositionalArguments();
	
};

template<class T>
ArgParser::Parameter::Parameter(const T& default_value, const char short_name, const std::string& description)
:short_name(short_name), description(description), set(false), position(0), config_file(false), help(false), remaining_parameters(false), remaining_parameter_min(-1), remaining_parameter_max(-1)
{
	std::ostringstream out;
	out<<default_value;
	this->default_value = out.str();
}

template<class T>
ArgParser::Parameter::Parameter(const T& default_value, const std::string& description)
:short_name('\0'), description(description), set(false), position(0), config_file(false), help(false), remaining_parameters(false), remaining_parameter_min(-1), remaining_parameter_max(-1)
{
	std::ostringstream out;
	out<<default_value;
	this->default_value = out.str();
}



template<typename T>
ArgParser::Parameter::operator T() const
{
	if(set)
	{
		T result;
		std::istringstream in(value);
		in>>result;
		if(in.fail())
			throw std::invalid_argument("Cannot convert value " + value + " to type " + std::string(typeid(T).name()));
		return result;
	}
	else if(!default_value.empty())
	{
		T result;
		std::istringstream in(default_value);
		in>>result;
		if(in.fail())
			throw std::invalid_argument("Cannot convert default value " + default_value + " to type " + std::string(typeid(T).name()));
		return result;
	}
	else
	{
		throw std::invalid_argument("Value is undefined");
	}
}

template<>
ArgParser::Parameter::operator bool() const;

template<>
ArgParser::Parameter::operator std::string() const;

template<typename T>
bool ArgParser::Parameter::operator ==(const T& v) const
{
	T v2 = this->operator T();
	return v2 == v;
}

typedef std::map<std::string, ArgParser::Parameter> ParameterSection;

//! Convert a ArgParser::Parameter to string
std::string str(const ArgParser::Parameter& p);

#endif
