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
  public:
    std::string GetFileName(Index index);
    void CreateIndexHead(Index index, short type);
    void Insert(Index index, char *key, int val);
    int Search(Index index, char *key, bool &exist);
    void Delete(Index index, char *key);
    void DeleteAll(Index index);
    void SearchLarger(Index index, char* key,vector<int> &valList, bool isEqual);
    void SearchSmaller(Index index, char *key, vector<int> &valList, bool isEqual);
    void DropIndexFile(Index index);
};

#endif