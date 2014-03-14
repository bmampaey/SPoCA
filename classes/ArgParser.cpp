#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <stdlib.h> 

#include "tools.h"
#include "ArgParser.h"

using namespace std;
ArgParser::Parameter::Parameter(const char short_name, const string& description)
:short_name(short_name), description(description), set(false), position(0), config_file(false), help(false), remaining_parameters(false), remaining_parameter_min(-1), remaining_parameter_max(-1)
{}

template<>
ArgParser::Parameter::Parameter(const bool& default_value, const char short_name, const string& description)
:short_name(short_name), description(description), set(false), position(0), config_file(false), help(false), remaining_parameters(false), remaining_parameter_min(-1), remaining_parameter_max(-1)
{
	this->default_value = default_value ? "true" : "false";
}

template<>
ArgParser::Parameter::Parameter(const bool& default_value, const string& description)
:short_name('\0'), description(description), set(false), position(0), config_file(false), help(false), remaining_parameters(false), remaining_parameter_min(-1), remaining_parameter_max(-1)
{
	this->default_value = default_value ? "true" : "false";
}


bool ArgParser::Parameter::is_set() const
{
	return set;
}

bool ArgParser::Parameter::is_positional() const
{
	return position != 0;
}

bool ArgParser::Parameter::is_config_file() const
{
	return config_file;
}

bool ArgParser::Parameter::is_help() const
{
	return help;
}

bool ArgParser::Parameter::is_remaining_parameters() const
{
	return remaining_parameters;
}


string ArgParser::Parameter::help_message(const string& long_name, string section, bool config_file, bool doxygen)
{
	string message;
	if(config_file)
	{
		if(!description.empty())
			message = "# " + description + "\n";
		if(default_value.empty())
		{
			message += long_name + " = ";
		}
		else
		{
			message += "#" + long_name + " = " + default_value;
		}
	}
	else if(doxygen)
	{
		message = "@param " + long_name;
		if(!description.empty())
			message += "\t" + description;
	}
	else if(is_positional() || is_remaining_parameters())
	{
		message += long_name + "\t" + description;
	}
	else
	{
		if(section == "global")
			section.clear();
		else
			section += ".";
		
		message += "--" + section + long_name;
		if(short_name)
			message += " | -" + section + short_name;
		if(!default_value.empty())
			message += "\tdefault: " + default_value;
		if(!description.empty())
			message += "\n\t" + description;
	}
	return message;
}

template<>
ArgParser::Parameter::operator bool() const
{
	string actual_value = set ? value : default_value;
	for (string::iterator it = actual_value.begin(); it != actual_value.end(); ++it)
		*it = static_cast<char>(tolower(static_cast<unsigned char>(*it)));
	
	if(actual_value.empty())
	{
		return set;
	}
	else
	{
		if(actual_value == "true" || actual_value == "t" || actual_value == "yes" || actual_value == "y")
			return true;
		else if(actual_value == "false" || actual_value == "f" || actual_value == "no" || actual_value == "n")
			return false;
		else
			throw invalid_argument("Cannot convert " + actual_value + " to bool");
	}
}

template<>
ArgParser::Parameter::operator string() const
{
	if(set)
	{
		return value;
	}
	else if(!default_value.empty())
	{
		return default_value;
	}
	else
	{
		throw std::invalid_argument("Value is undefined");
	}
}

bool ArgParser::Parameter::operator ==(const char* v) const
{
	string v2 = this->operator string();
	return v2 == v;
}

ostream& operator<<(ostream& os, const ArgParser::Parameter& parameter)
{
	if(parameter.set)
	{
		os << parameter.value;
	}
	else if(!parameter.default_value.empty())
	{
		os << parameter.default_value;
	}
	else
	{
		throw invalid_argument("Value is undefined");
	}
	return os;
}

bool compare_parameter_position(const ArgParser::Parameter* c1, const ArgParser::Parameter* c2 )
{
	return c1->position < c2->position;
}


bool trim(string& s)
{
	size_t pos = s.find_first_not_of(" \t");
	if(pos != string::npos)
		s.erase(0, pos);
	else
		s.clear();
	return true;
}

ArgParser::ArgParser(string description)
:description(description)
{
	sections["global"];
}


string ArgParser::section_help_message(const string& section, vector<pair<unsigned, string> >& positional_parameters, string& remaining_parameters_name, bool config_file, bool doxygen)
{
	string config_files_message, help_message, other_message;
	for(ParameterSection::iterator parameter = sections[section].begin(); parameter != sections[section].end(); ++parameter)
	{
		if(parameter->second.is_positional())
		{
			positional_parameters.push_back(make_pair(parameter->second.position, parameter->first));
		}
		else if(parameter->second.is_remaining_parameters())
		{
			remaining_parameters_name = parameter->first;
		}
		else if(parameter->second.is_help())
		{
			help_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
		else if(parameter->second.is_config_file())
		{
			config_files_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
		else
		{
			other_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
	}
	sort(positional_parameters.begin(), positional_parameters.end());
	
	string message = config_file ? "# "+ section + " parameters\n"+ section + ":" : section + " parameters:";
	
	if(!help_message.empty() && !config_file)
		message += help_message;
	if(!config_files_message.empty() && !config_file)
		message += config_files_message;
	if(!other_message.empty())
		message += other_message;
	
	return message;
}

string ArgParser::section_help_message(const string& section, bool config_file, bool doxygen)
{
	string config_files_message, help_message, other_message;
	for(ParameterSection::iterator parameter = sections[section].begin(); parameter != sections[section].end(); ++parameter)
	{
		if(parameter->second.is_positional() || parameter->second.is_remaining_parameters())
		{
			continue;
		}
		else if(parameter->second.is_help())
		{
			help_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
		else if(parameter->second.is_config_file())
		{
			config_files_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
		else
		{
			other_message += "\n\n" + parameter->second.help_message(parameter->first, section, config_file, doxygen);
		}
	}
	
	string message = config_file ? "# "+ section + " parameters\n"+ section + ":" : section + " parameters:";
	
	if(!help_message.empty() && !config_file)
		message += help_message;
	if(!config_files_message.empty() && !config_file)
		message += config_files_message;
	if(!other_message.empty())
		message += other_message;
	
	return message;
}

string ArgParser::help_message(const string& executable, bool config_file, bool doxygen)
{
	vector<pair<unsigned, string> > positional_parameters;
	string remaining_parameters_name;
	string nominals;

	for(SectionList::iterator section = sections.begin(); section != sections.end(); ++section)
	{
		if(section->first != "global")
		{
			nominals += "\n\n" + section_help_message(section->first, config_file, doxygen);
		}
		else
		{
			nominals = section_help_message("global", positional_parameters, remaining_parameters_name, config_file, doxygen) + nominals;
		}
	}
	
	string message;
	if(config_file)
	{
		message = "#Configuration file for the program " + executable;
		message += "\n\n" + nominals;
	}
	else
	{
		string positionals;
		string positional_args = " ";
		for(vector<pair<unsigned, string> >::const_iterator parameter = positional_parameters.begin(); parameter != positional_parameters.end(); ++parameter)
		{
			positional_args += " " + parameter->second;
			if(! sections["global"][parameter->second].description.empty())
				positionals += "\n" + sections["global"][parameter->second].help_message(parameter->second, "global", config_file, doxygen);
		}
		if(! remaining_parameters_name.empty())
		{
			positionals += "\n" + sections["global"][remaining_parameters_name].help_message(remaining_parameters_name, "global", config_file, doxygen);
			int i = 0;
			for(; i < sections["global"][remaining_parameters_name].remaining_parameter_min; ++i)
				positional_args += " " + remaining_parameters_name;
			
			if(sections["global"][remaining_parameters_name].remaining_parameter_max > sections["global"][remaining_parameters_name].remaining_parameter_min)
			{
				positional_args += " [";
				for(; i < sections["global"][remaining_parameters_name].remaining_parameter_max; ++i)
					positional_args += " " + remaining_parameters_name;
				positional_args += " ]";
			}
			else if(sections["global"][remaining_parameters_name].remaining_parameter_max < 0)
			{
				positional_args += " [ " + remaining_parameters_name + " ... ]";
			}
		}
		
		if (!description.empty())
			message = description + "\n";
		
		if(doxygen)
		{
			message += "@section usage Usage\n<tt> " + executable + " [-option optionvalue ...]" + positional_args;
			message += " </tt>";
			message += "\n" + positionals;
			message += "\n\n" + nominals;
		}
		else
		{
			message += "Usage: " + executable + " [-option optionvalue ...]" + positional_args;
			message += "\nwhere:" + positionals;
			message += "\n\n" + nominals;
		}
	}
	return message;
}

deque<string> ArgParser::RemainingPositionalArguments()
{
	return positional_arguments;
}

ParameterSection& ArgParser::operator() (const string& index)
{
	return sections[index];
}

ArgParser::Parameter& ArgParser::operator[] (const string& index)
{
	return sections["global"][index];
}



void ArgParser::parse(const int argc, const char **argv, bool warn_duplicate, bool overwrite)
{
	string current_section = "global";
	string parameter;
	string section;
	bool only_positional = false;
	
	vector<vector<string> > nominal_arguments;
	
	positional_arguments.clear();
	
	for(int i = 1; i < argc; ++i)
	{
		string argument(argv[i]);
		if(only_positional)
		{
			positional_arguments.push_back(argument);
		}
		else if(parse_positional_only(argument))
		{
			// All arguments parameters will be positional arguments
			only_positional = true;
		}
		else if(parse_section_parameter(argument, section))
		{
			// We have a section
			if(!sections.count(section))
			{
				throw invalid_argument("Unknown section " + section);
			}
			current_section = section;
			section.clear();
		}
		else if(parse_long_parameter(argument, parameter, section))
		{
			// We have a long parameter
			section = section.empty() ? current_section : section;
			if(!sections.count(section))
			{
				throw invalid_argument("Unknown section " + section);
			}
			if(!sections[section].count(parameter))
			{
				throw invalid_argument("Unknown parameter " + parameter + " for " + section + " section");
			}
			string sp[] = {section, parameter};
			nominal_arguments.push_back(vector<string>(sp, sp + 2));
			section.clear();
		}
		else if(parse_short_parameter(argument, parameter, section))
		{
			// We have a short parameter
			section = section.empty() ? current_section : section;
			if(!sections.count(section))
			{
				throw invalid_argument("Unknown section " + section);
			}
			string long_name = find_long_parameter_name(parameter[0], section);
			if(long_name.empty())
			{
				throw invalid_argument("Unknown short parameter " + parameter + " for " + section + " section");
			}
			string sp[] = {section, long_name};
			nominal_arguments.push_back(vector<string>(sp, sp + 2));
		}
		else if(! parameter.empty())
		{
			// We have a value for a parameter
			nominal_arguments.back().push_back(argument);
			parameter.clear();
		}
		else
		{
			// We have a positional argument
			positional_arguments.push_back(argument);
		}
	}
	#if defined VERBOSE
	cout<<"Positional arguments:"<<endl;
	for(deque<string>::iterator i = positional_arguments.begin(); i != positional_arguments.end(); ++i)
	{
		cout<<"\t"<<*i<<endl;
	}
	cout<<"Nominal arguments:"<<endl;
	for(vector<vector<string> >::iterator i = nominal_arguments.begin(); i != nominal_arguments.end(); ++i)
	{
		if(i->size() >= 3)
			cout<<"\t"<<i->at(0)<<": "<<i->at(1)<<" = "<<i->at(2)<<endl;
		else
			cout<<"\t"<<i->at(0)<<": "<<i->at(1)<<endl;
		
	}
	#endif
	
	// We keep track of configuration files arguments
	deque<Parameter*> config_files_parameters;
	
	// We keep track of help arguments
	Parameter* help_parameter = NULL;
	
	// We set the nominal parameters value
	for(vector<vector<string> >::iterator i = nominal_arguments.begin(); i != nominal_arguments.end(); ++i)
	{
		if(sections[i->at(0)][i->at(1)].is_set())
		{
			if(warn_duplicate)
			{ 
				cerr<<"WARN: Argument "<<i->at(0)<<"."<<i->at(1)<<" was set more than once"<<endl;
			}
			if(! overwrite)
			{
				continue;
			}
		}
		sections[i->at(0)][i->at(1)].set = true;
		if(i->size() >= 3)
			sections[i->at(0)][i->at(1)].value = i->at(2);
		if(sections[i->at(0)][i->at(1)].is_config_file())
			config_files_parameters.push_back(&(sections[i->at(0)][i->at(1)]));
		if(sections[i->at(0)][i->at(1)].is_help())
			help_parameter = &(sections[i->at(0)][i->at(1)]);
	}
	
	// We check the help arguments
	if(help_parameter)
	{
		if(help_parameter->value == "config")
		{
			cout<<help_message(argv[0], true, false)<<endl;
			exit(EXIT_SUCCESS);
		}
		else if(help_parameter->value == "doxygen")
		{
			cout<<help_message(argv[0], false, true)<<endl;
			exit(EXIT_SUCCESS);
		}
		else if(help_parameter->value.empty())
		{
			cout<<help_message(argv[0])<<endl;
			exit(EXIT_SUCCESS);
		}
		else
		{
			throw invalid_argument("Unknow help type "+ help_parameter->value + " requested");
		}
	}
	
	// We look for the positional arguments (we only accept positional argument for the global section)
	vector<Parameter*> positional_parameters;
	string remaining_parameters_name;
	for(ParameterSection::iterator parameter = sections["global"].begin(); parameter != sections["global"].end(); ++parameter)
	{
		if(parameter->second.is_positional())
			positional_parameters.push_back(&(parameter->second));
		else if(parameter->second.is_remaining_parameters())
		{
			if(remaining_parameters_name.empty())
			{
				remaining_parameters_name = parameter->first;
			}
			else
			{
				throw invalid_argument("More than one Remaining Positional Parameters declared: " + remaining_parameters_name + " and " + parameter->first);
			}
		}
	}
	sort(positional_parameters.begin(), positional_parameters.end(), compare_parameter_position);
	
	// We set the positional parameters value
	for(unsigned i = 0; i < positional_parameters.size() && !positional_arguments.empty(); ++i)
	{
		positional_parameters[i]->set = true;
		positional_parameters[i]->value = positional_arguments.front();
		positional_arguments.pop_front();
	}
	
	// We verify the remaining 
	if(! remaining_parameters_name.empty())
	{
		if(int(positional_arguments.size()) < sections["global"][remaining_parameters_name].remaining_parameter_min)
		{
			// We have less remaining positional arguments than requested
			throw invalid_argument("You must specify at least " + itos(sections["global"][remaining_parameters_name].remaining_parameter_min) + " " + remaining_parameters_name);
		}
		else if(sections["global"][remaining_parameters_name].remaining_parameter_max > 0 && int(positional_arguments.size()) > sections["global"][remaining_parameters_name].remaining_parameter_max)
		{
			// We have more remaining positional arguments than allowed
			throw invalid_argument("You must specify at most " + itos(sections["global"][remaining_parameters_name].remaining_parameter_max) + " " + remaining_parameters_name);
		}
	}
	else
	{
		// We have remaining positional arguments that were not declared
		throw invalid_argument("The following positional arguments are superfluous: " + to_string(positional_arguments));
	}
	
	// We parse the config files
	while(! config_files_parameters.empty())
	{
		parse_file(config_files_parameters.front()->value, warn_duplicate, false);
		config_files_parameters.pop_front();
	}
}

void ArgParser::parse_file(const string& filename, bool warn_duplicate, bool overwrite)
{
	string current_section = "global";
	string parameter;
	string section;
	string value;
	string argument;
	vector<vector<string> > nominal_arguments;
	
	ifstream config_file(filename.c_str());
	if(! config_file.is_open())
	{
		throw invalid_argument("Cannot open file " + filename);
	}
	int line_number = 0;
	while(getline(config_file, argument))
	{
		++line_number;
		trim(argument);
		if(parse_section(argument, section))
		{
			// We have a section
			if(sections.count(section))
			{
				current_section = section;
				section.clear();
			}
			else
			{
				throw invalid_argument("Unknown section " + section + " on line " + itos(line_number));
			}
		}
		else if(parse_option(argument, parameter, section, value))
		{
			// We have an option
			section = section.empty() ? current_section : section;
			if(!sections.count(section))
			{
				throw invalid_argument("Unknown section " + section + " on line " + itos(line_number) + " in file " + filename);
			}
			if(!sections[section].count(parameter))
			{
				throw invalid_argument("Unknown parameter " + parameter + " for " + section + " section on line " + itos(line_number) + " in file " + filename);
			}
			string spv[] = {section, parameter, value};
			nominal_arguments.push_back(vector<string>(spv, spv + 3));
			section.clear();
			parameter.clear();
			value.clear();
		}
		if(trim(argument) && ! argument.empty() && ! parse_comment(argument))
		{
			// We have an error
			throw invalid_argument("Invalid line number " + itos(line_number) + " in file " + filename);
		}
	}
	
	#if defined VERBOSE
	cout<<"Config file "<<filename<<" options:"<<endl;
	for(vector<vector<string> >::iterator i = nominal_arguments.begin(); i != nominal_arguments.end(); ++i)
	{
		cout<<"\t"<<i->at(0)<<": "<<i->at(1)<<" = "<<i->at(2)<<endl;
	}
	#endif
	
	// We keep track of configuration files arguments
	deque<Parameter*> config_files_parameters;
	
	// We set the nominal parameters value
	for(vector<vector<string> >::iterator i = nominal_arguments.begin(); i != nominal_arguments.end(); ++i)
	{
		if(sections[i->at(0)][i->at(1)].is_set())
		{
			if(warn_duplicate)
			{ 
				cerr<<"WARN: Argument "<<i->at(0)<<"."<<i->at(1)<<" was set more than once"<<endl;
			}
			if(! overwrite)
			{
				continue;
			}
		}
		sections[i->at(0)][i->at(1)].set = true;
		if(i->size() >= 3)
			sections[i->at(0)][i->at(1)].value = i->at(2);
		if(sections[i->at(0)][i->at(1)].is_config_file())
			config_files_parameters.push_back(&(sections[i->at(0)][i->at(1)]));
	}
	
	
	// We parse the config files
	while(! config_files_parameters.empty())
	{
		parse_file(config_files_parameters.front()->value, warn_duplicate, overwrite);
		config_files_parameters.pop_front();
	}
}

bool ArgParser::parse_positional_only(string& argument)
{
	if(argument == "--")
	{
		argument.clear();
		return true;
	}
	return false;
}

bool ArgParser::parse_section_parameter(string& argument, string& section)
{
	if(argument.size() >= 3 && argument[0] == '-' && argument[1] == '-')
	{
		string argument_backup = argument;
		argument.erase(0, 2);
		if(parse_section_id(argument, section) && argument == ":")
		{
			argument.clear();
			return true;
		}
		else
		{
			section.clear();
			argument = argument_backup;
		}
	}
	return false;
}

bool ArgParser::parse_comment(string& argument)
{
	if(argument[0] == '#' || argument[0] == ';')
	{
		argument.clear();
		return true;
	}
	return false;
}

bool ArgParser::parse_value(string& argument, string& value)
{
	if(argument.empty())
	{
		return false;
	}
	if(argument[0] == '"' || argument[0] == '\'')
	{
		char quote = argument[0];
		for(unsigned i = 1; i < argument.size(); ++i)
		{
			if(argument[i] == '\\')
			{
				if(i+1 < argument.size())
				{
					if(argument[i+1] == quote)
					{
						value.push_back(argument[i+1]);
						++i;
					}
					else
					{
						value.push_back(argument[i]);
					}
				}
				else
				{
					return false;
				}
			}
			else if(argument[i] == quote)
			{
				argument.erase(0, i+1);
				return true;
			}
			else
			{
				value.push_back(argument[i]);
			}
		}
	}
	else
	{
		size_t pos = argument.find_first_of(";#");
		if(pos != string::npos)
		{
			pos = argument.substr(0, pos).find_last_not_of(" \t");
			if(pos != string::npos)
			{
				value = argument.substr(0, pos+1);
				argument.erase(0, pos+1);
				return true;
			}
		}
		else
		{
			pos = argument.find_last_not_of(" \t");
			if(pos != string::npos)
			{
				value = argument.substr(0, pos+1);
				argument.erase(0, pos+1);
				return true;
			}
		}
	}
	value.clear();
	return false;
}


bool ArgParser::parse_section(string& argument, string& section)
{
	string argument_backup = argument;
	if(argument[0] == '[')
	{
		argument.erase(0, 1);
		if(parse_section_id(argument, section) && trim(argument) && argument[0] == ']')
		{
			argument.erase(0, 1);
			return true;
		}
		else
		{
			section.clear();
			argument = argument_backup;
		}
	}
	else if(parse_section_id(argument, section) && trim(argument) && argument[0] == ':')
	{
		argument.erase(0, 1);
		return true;
	}
	else
	{
		section.clear();
		argument = argument_backup;
	}
	return false;
}



bool ArgParser::parse_long_parameter(string& argument, string& parameter, string& section)
{
	if(argument.size() >= 3 && argument[0] == '-' && argument[1] == '-')
	{
		string argument_backup = argument;
		argument.erase(0, 2);
		if(parse_section_prefix(argument, section) && parse_parameter_id(argument, parameter) && argument.empty())
		{
			return true;
		}
		else
		{
			section.clear();
			parameter.clear();
			argument = argument_backup;
		}
		argument.erase(0, 2);
		if(parse_parameter_id(argument, parameter) && argument.empty())
		{
			section.clear();
			return true;
		}
		else
		{
			section.clear();
			parameter.clear();
			argument = argument_backup;
		}
	}
	return false;
}

bool ArgParser::parse_option(string& argument, string& parameter, string& section, string& value)
{
	string argument_backup = argument;
	if(parse_section_prefix(argument, section) && parse_parameter_id(argument, parameter) && argument.find_first_of("= \t") == 0)
	{
		size_t pos = argument.find_first_not_of("= \t");
		argument.erase(0, pos);
		if(parse_value(argument, value))
		{
			return true;
		}
	}
	section.clear();
	parameter.clear();
	value.clear();
	argument = argument_backup;
	
	if(parse_parameter_id(argument, parameter) && argument.find_first_of("= \t") == 0)
	{
		size_t pos = argument.find_first_not_of("= \t");
		argument.erase(0, pos);
		if(parse_value(argument, value))
		{
			return true;
		}
	}
	section.clear();
	parameter.clear();
	value.clear();
	argument = argument_backup;
	return false;
}


bool ArgParser::parse_short_parameter(string& argument, string& parameter, string& section)
{
	if(argument.size() >= 2 && argument[0] == '-')
	{
		string argument_backup = argument;
		argument.erase(0, 1);
		if(parse_section_prefix(argument, section) && parse_parameter_id(argument, parameter) && parameter.size() == 1 && argument.empty())
		{
			return true;
		}
		else
		{
			section.clear();
			parameter.clear();
			argument = argument_backup;
		}
		argument.erase(0, 1);
		if(parse_parameter_id(argument, parameter) && parameter.size() == 1 && argument.empty())
		{
			section.clear();
			return true;
		}
		else
		{
			section.clear();
			parameter.clear();
			argument = argument_backup;
		}
	}
	return false;
}


bool ArgParser::parse_section_prefix(string& argument, string& section)
{
	string argument_backup = argument;
	if(parse_section_id(argument, section) && !argument.empty() && argument[0] == '.')
	{
		argument.erase(0, 1);
		return true;
	}
	else
	{
		section.clear();
		argument = argument_backup;
	}
	return false;
}

bool ArgParser::parse_parameter_id(string& argument, string& identifier)
{
	if(isalpha(argument[0]))
	{
		size_t pos = argument.find_first_not_of(alpha_num_under);
		identifier = argument.substr(0, pos);
		argument.erase(0, pos);
		return true;
	}
	return false;
}

bool ArgParser::parse_section_id(string& argument, string& identifier)
{
	if(!argument.empty() && isalpha(argument[0]))
	{
		size_t pos = argument.find_first_not_of(alpha_num_under);
		identifier = argument.substr(0, pos);
		argument.erase(0, pos);
	}
	else
	{
		identifier = "global";
	}
	return true;
}

string ArgParser::find_long_parameter_name(const char short_name, const string section)
{
	for(ParameterSection::const_iterator parameter=sections[section].begin(); parameter != sections[section].end(); ++parameter)
	{
		if(parameter->second.short_name == short_name)
			return parameter->first;
	}
	return "";
}

const string ArgParser::alpha_num_under = "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789_";
unsigned ArgParser::positional_counter = 1;

ArgParser::Parameter ArgParser::PositionalParameter(const string& description)
{
	Parameter parameter;
	parameter.position = positional_counter;
	parameter.description = description;
	++positional_counter;
	return parameter;
}

ArgParser::Parameter ArgParser::RemainingPositionalParameters(const string& description, unsigned min, int max)
{
	Parameter parameter;
	parameter.remaining_parameters = true;
	parameter.remaining_parameter_min = min;
	parameter.remaining_parameter_max = max;
	parameter.description = description;
	return parameter;
}

ArgParser::Parameter ArgParser::ConfigurationFile(const string& description, const string& default_value)
{
	Parameter parameter(default_value, description);
	parameter.config_file = true;
	return parameter;
}
ArgParser::Parameter ArgParser::ConfigurationFile(const char short_name, const string& description, const string& default_value)
{
	Parameter parameter(default_value, short_name, description);
	parameter.config_file = true;
	return parameter;
}

ArgParser::Parameter ArgParser::Help(const char short_name)
{
	Parameter parameter(short_name, "Print a help message and exit. If you pass the value doxygen, the help message will follow the doxygen convention. If you pass the value config, the help message will write a configuration file template.");
	parameter.help = true;
	return parameter;
}

string str(const ArgParser::Parameter& p)
{
	string result = p;
	return result;
}

