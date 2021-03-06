#ifndef __INDEX_
#define __INDEX_

#include<iostream>
#include "BufferManager.h"
#include "BpTree.h"
#include "Minisql.h"

#define MaxOffsetPos_IM 0
#define TypePos_IM 4
#define RootPos_IM 6
extern BufferManager bf;


class IndexManager{
  private:
    void GetInterval(std::string name, char *data, short start, short end, short type, vector<int> &valList);
    void SearchLarger(std::string name, int offset, char *key,short type, vector<int> &valList, bool isEqual);
    void SearchSmaller(std::string name, int offset, char *key, short type, vector<int> &valList, bool isEqual);
    std::string GetFileName(Index index);
  public:
    void CreateIndexHead(Index index, short type);
    int Insert(Index index, char *key, int val);
    int Search(Index index, char *key, bool &exist);
    int Delete(Index index, char *key);
    void DeleteAll(Index index);
    void SearchLarger(Index index, char* key,vector<int> &valList, bool isEqual);
    void SearchSmaller(Index index, char *key, vector<int> &valList, bool isEqual);
    void DropIndexFile(Index index);
};

#endif