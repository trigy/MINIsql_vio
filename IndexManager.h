#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_

#include "Minisql.h"
#include "bplus.h"
using namespace std;

class Value_offset{
public:
	string s;
	int off;
};

//automatically create index on primary key
void CreateIndex(string index_name, int attr_type);//0 int,1 float,2 string
//CREATE INDEX NAME ON NAME '(' NAME ')'

void CreateIndex(vector<Value_offset> values, string index_name, int attr_type);

void InsertIndex(string index_name, string value, int offset);

void DeleteIndex(string index_name, string value);

vector<int> Find_indices(string index_name, string op,string value);

void disre(node *p, int q,string filename);

void dis(string filename);

#endif

