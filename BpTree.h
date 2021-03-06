#ifndef __BPTREE_
#define __BPTREE_

#include<iostream>
#include "BufferManager.h"
// #include"key.h"
#define KEY const char*

#define tipNum          1000

#define ValidPos        0
#define TypePos         1
#define WidthPos        2
#define StorePos        4
#define ParentPos       6
#define BlockAttSpace   10

#define KeyLength       (type > 0 ? type : 4)

class BpTree{
  private:
    std::string name;
    int offset;
    int blockNum;
    short type;
    int InsertInThisNode(KEY key, int val);
  public: 
    char *data;
    BpTree(std::string name, int offset, int blockNum, short type);
    int BlockNum();
    int Offset();
    int InsertToLeaf(KEY key, int val);
    int Split(short index);
    bool Search(KEY key,short &index);
    int SearchKey(KEY key, bool &exist);
    int DeleteKey(KEY key);
    int Reform();
};

#endif