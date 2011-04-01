#pragma once
#ifndef Table_H
#define Table_H

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <typeinfo>

class Table
{

	private:
		std::string table_name;
		std::vector<char*> columns_name;
		std::vector<type_info> columns_data_type;
		std::vector<void *> columns_data;
		std::vector<char*> columns_unit;
		std::vector<unsigned> row_number;
		unsigned current_column;
	public:
		//Constructors and destructors
		Table(const std::string& name, unsigned number_rows)
		:name(name),number_rows(number_rows),previous_column(0){}
		
		//Accessors
		char* Name()
		{return const_cast<char *>(name.c_str())}
		char ** ColumnsName()
		{
			return char**(&columns_name[0]);
		}
		void* Data(unsigned i)
		{
			return columns_data.at(i);
		}
		type_info DataType(unsigned i)
		{	
			return fitsDataType(columns_data_type.at(i));
		}
		//Routine to add a column
		template<T>
		bool addColumn(const char* name, const char* unit = NULL)
		{
			// I need to verify if the column already exist
			bool found = false;
			for(unsigned c = 0; c < columns_name.size() && !found; ++c)
			{
				if(columns_name[current_column] == name)
					found = true;
				else
					current_column = (current_column + 1) % columns_name.size();
			}
			if(!found)
			{
				char * namecp = new char[sizeof(name)/sizeof(name[0])];
				strcpy (namecp, name);
				columns_name.push(namecp);
				if(unit != NULL)
				{
					char * unitcp = new char[sizeof(unit)/sizeof(unit[0])];
					strcpy (unitcp, unit);
					columns_unit.push(unitcp);
				}
				else
					columns_unit.push(NULL);
				columns_data_type.push(typeid(T));
				columns_data.push((void *)new T[number_rows]);
				row_number.push(0);
				current_column = columns_name.size() -1;
			}
			else if(columns_data_type[current_column] != typeid(T))// I check that the types match
			{
				cerr<<"Error trying to add column "<<name<<" with type "<< typeid(T).name()<<", column of same name already exist with type "<< columns_data_type[current_column].name()<<endl;
				return false;
			}
		}
		//Routine to add data
		template<T>
		bool addData(const char* name, T data, const char* unit = NULL)
		{
			addColumn<T>(name, unit)
			if(row_number[col_number] == number_rows - 1)
			{
				cerr<<"Error trying to add data to column "<<name<<", no more space."<<endl;
				return false;
			}
			columns_data[col_number][row_number[col_number]] = data;
			++row_number[col_number];

		}
		friend ostream& operator<<(ostream& out, const Table& T);
};

ostream& operator<<(ostream& out, const Table& T)
{
	out<<"Not implemented yet!"<<endl;
	return out;
}


#endif
