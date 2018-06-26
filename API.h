// #pragma once
#ifndef __API_H_
#define __API_H_

#include <ctime>
#include <algorithm>
#include "Minisql.h"
#include "IndexManager.h"
#include "CatalogManager.h"
#include "RecordManager.h"

extern RecordManager RCM;
extern CatalogManager CTM;
extern IndexManager IDM;
extern BufferManager bf;

using namespace std;

void API_CreateTable(Table &table);

void API_DropTable(string table_name);

void API_CreateIndex(Index& index);

void API_DropIndex(Index index);

void API_Insert(string table_name, string Values); //error

//inside function when operate on the log 
void API_Select(string table_name, vector<string> & attr, string WClause);

void API_Delete(string table_name,string WClasue);

string split(string str,string pattern,int id);

vector<string> split(string str, string pattern);

void API_DrawResult(Table& tb,Record res);

//merry the time
double Cal_time(long start, long end);

#endif
