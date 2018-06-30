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

// void API_DropIndex(Index index);
void API_DropIndex(string table_name, string index_name);

// void API_Insert(string table_name, string Values); //error

void API_Insert(Table& table, Record& record);

//inside function when operate on the log 
// void API_Select(string table_name, vector<string> & attr, string WClause);

void API_SelectAll(Table& table, vector<string>& selectAttr);

void API_SelectCon(Table& table, vector<string>& selectAttr, ConditionList& cl);

// void API_Delete(string table_name,string WClasue);

void API_DeleteAll(Table &table);

void API_DeleteCon(Table &table, ConditionList &cl);

// string split(string str,string pattern,int id);

// vector<string> split(string str, string pattern);

// void API_DrawResult(Table& tb,Record res);

// merry the time
// double Cal_time(long start, long end);

#endif
