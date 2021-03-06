#ifndef __CLASS_
#define __CLASS_

#include <iostream>
#include <string>
#include <vector>

#define INT 0
#define FLOAT -1

#define PRIMARY 0
#define UNIQUE  1
#define OTHER   2

#define EQU 0
#define NEQ 1
#define LGE 2
#define SME 3
#define LRG 4
#define SML 5 

using namespace std;
class Attribute
{
public:
	string attr_name;
	int attr_type;     //0:INT, -1:FLOAT OTHER:CHAR
    // char represent the length
	int attr_key_type; //0:PRIMARY, 1:UNIQUE, 2:OTHER
    int attr_id;        //atribute's no.
public:
  Attribute(){}
  Attribute(string a, int b, int c, int d)
  {
	  attr_name = a;
	  attr_type = b;
	  attr_key_type = c;
	  attr_id = d;
	  }
	~Attribute(){}	
};

class Table
{
public:
	string table_name;
	int attr_num;         //number of the attribute
	vector<Attribute> attrs;
public:
	Table(const Table& t)
	{
		table_name=t.table_name;
		attr_num=t.attr_num;
		for(int i=0;i<t.attrs.size();i++)
		{
			attrs.push_back(t.attrs[i]);
		}
	}
	Table(){}
	~Table(){}
	int getPrimaryKeyId() //if no primary key return -1;
	{
		for (int i = 0; i < attr_num; i++)
		{
			if (attrs[i].attr_key_type == PRIMARY)
				return i;
		}
		return -1;
	}
	int searchAttrId(string AttrName)//if no primary key return -1
	{
		for (int i = 0; i < attr_num; i++)
		{
			if (attrs[i].attr_name == AttrName)
				return i;
		}
		return -1;
	}
	int length()
	{
		int s = 0;
		for (int i = 0; i < attr_num; i++)
		{
			s += attrs[i].attr_type;
		}
		return s;
	}
};

class Condition
{
public:
	string attr_name; //attr's name of the contion
	int operation; //can read 0: "=", 1: "<>", 2: ">=", 3: "<=",4 :">", 5: "<"
	char cmp_value[256] ; //compared object
	Condition(){}
};
typedef vector<Condition> ConditionList;
class Index     //explain the search information
{
public:
	string index_name;
	string table_name;
	string attr_name;
public:
	Index(){}
	Index(string a, string b, string c)
	{
		index_name = a;
		table_name = b;
		attr_name = c;
	}
	~Index(){}
};


class Record{
  public:
	char atts[32][256];
	vector<bool> null;
};  

static char bitMap[4]={
	0x8, 0x4, 0x2, 0x1
};
#endif
