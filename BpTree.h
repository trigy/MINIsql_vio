#ifndef __BPTREE_
#define __BPTREE_

#include<iostream>
#include "BufferManager.h"
// #include"key.h"
#define KEY const char*

#define tipNum          10

#define ValidPos        0
#define TypePos         1
#define WidthPos        2
#define StorePos        4
#define ParentPos       6
// #define NextPos         10
// #define BlockAttSpace   14
#define BlockAttSpace   10

#define KeyLength       (type > 0 ? type : 4)

class BpTree{
  private:
    std::string name;
    int offset;
    int blockNum;
    short type;
    char *data;
  public: 
    BpTree(std::string name, int offset, int blockNum, short type);
    int BlockNum();
    int Offset();
    int InsertToLeaf(KEY key, int val);
    int InsertInThisNode(KEY key, int val);
    int Split(int index);
    bool Search(KEY key,int &index);
    int SearchKey(KEY key);
    int DeleteKey(KEY key);
    int Reform();
};

#endif