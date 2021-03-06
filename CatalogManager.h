#ifndef __CATA_H_
#define __CATA_H_

#include<iostream>
#include"BufferManager.h"
#include"Minisql.h"

#define AttNumPos     0
#define PrimaryNumPos 2
#define UniquePos     4
#define TableAttPos   8

#define IndexNumPos   0
#define IndexAttPos   2

#define WordMaxLength 20

class CatalogManager{ 
  public:
    std::string GetFileName(std::string tableName);

    bool TableExist(std::string name);
    Table ReadTable(std::string name);
    int CreateTable(Table table);
    bool IndexInit(std::string name);
    void DropTable(Table table);

    int IndexOffset(std::string tableName, std::string indexName);
    int IndexOffset2(std::string tableName, std::string attrName);
    Index ReadIndex(std::string tableName, std::string attrName, int indexOffset);
    int GetIndexList(Table table, vector<Index> &indexList);
    void CreateIndex(Index index);
    void DropIndex(std::string tableName, int indexOffset);
};

#endif