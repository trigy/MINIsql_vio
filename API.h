#pragma once
#include <ctime>
#include "Minisql.h"
#include "IndexManager.h"
#include "CatalogManager.h"
#include "RecordManager.h"

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
