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
void API_DropIndex(string table_name, string index_name);
void API_Insert(Table& table, Record& record);
void API_SelectAll(Table& table, vector<string>& selectAttr);
void API_SelectCon(Table& table, vector<string>& selectAttr, ConditionList& cl);
void API_DeleteAll(Table &table);
void API_DeleteCon(Table &table, ConditionList &cl);

#endif
