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
  public:
    std::string GetFileName(Index index);
    void CreateIndexHead(Index index, short type);
    void Insert(Index index, char *key, int val);
    int Search(Index index, char *key, bool &exist);
    void Delete(Index index, char *key);
};

#endif