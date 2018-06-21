#include <iostream>
#include <string>
#include <vector>
#include <list>

//Attribute's len has some problem
using namespace std;
class Attribute
{
public:
	string attr_name;
	int attr_type;     //0:INT, -1:FLOAT OTHER:CHAR
    // char represent the length
	int attr_key_type; //0:PRIMARY, 1:UNIQUE, 2:NULL(EMPTY)
//	int attr_len;	   //CHAR:0. not char:1
    int attr_id;        //atribute's no.
public:
    Attribute(string a, int b,int c, int d, int e){attr_name = a;attr_type = b;attr_key_type = c; attr_len = d;attr_id = e;};
	~Attribute();	
};

class Table
{
public:
	string table_name;
	int attr_num;         //number of the attribute
	vector<Attribute> attrs;
public:
	Table();
	~Table();
	int getPrimaryKeyId(); //if no primary key return -1;
	int searchAttrId(string AttrName);//if no primary key return -1
	int length();
};

class Condition
{
public:
	string attr_name; //attr's name of the contion
	string operation; //can read "<>","=",">=""<="">""<"
	string cmp_value; //compared object
	Condition(string a, string b, string c)
	{
		attr_name = a;
		operation = b;
		cmp_value = c;
	}
};
typedef list<Condition> ConditionList;
class Index     //explain the search information
{
public:
	string index_name;
	string table_name;
	string attr_name;
public:
	Index(string a, string b, string c)
	{
		index_name = a;
		table_name = b;
		attr_name = c;
	}
	~Index();
};

struct Tuple: public Table
{
    string attr_values[32];    
};

int Table::getPrimaryKeyId()
{
	for(int i = 0; i < attr_num; i++)
	{
		if(attrs[i].attr_key_type == PRIMARY)
			return i;
	}
	return -1;
}

int Table::searchAttrId(string AttrName)
{
	for(int i = 0; i < attr_num; i++)
	{
		if(attrs[i].attr_name == AttrName)
			return i;
	}
	return -1;
}

int Table::length()
{
	int s = 0;
	for(int i = 0; i < attr_num; i++)
	{
		s += attrs[i].attr_len;
	}
	return s;
}





