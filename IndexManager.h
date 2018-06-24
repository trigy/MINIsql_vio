#include<iostream>
#include "BufferManager.h"
#include "BpTree.h"
#include "Minisql.h"
// #include"key.h"
#define KEY char*
#define MaxOffsetPos 0
#define TypePos 4
#define RootPos 6
extern BufferManager bf;

class IndexManager{
  public:
    std::string GetFileName(Index index);
    void CreateIndexHead(Index index, short type);
    void Insert(Index index, std::string key, int val);
    int Search(Index index, std::string key);
    void Delete(Index index, std::string key);
};
std::string IndexManager::GetFileName(Index index)
{
  return index.index_name + "-" + index.table_name + ".idx";
}

void IndexManager::CreateIndexHead(Index index, short type)
{
  std::string name=GetFileName(index);
  int blockNum=bf.AddNewBlockToFile(name,0);
  char newData[BlockMaxSize];
  *(bool*)(newData)=true;
  *(int*)(newData+TypePos)=type;
  *(int*)(newData+RootPos)=0;
  bf.WriteData(blockNum,newData,0,BlockMaxSize);
}

void IndexManager::Insert(Index index, std::string key, int val)
{
  std::string name=GetFileName(index);
  int blockNum=bf.FindBlock(name,0);
  char* fileHead=bf.ReadBlockData(blockNum);
  short type=*(short*)(fileHead+TypePos);
  int rootOffset,rootNum;
  rootOffset=*(int*)(fileHead+RootPos);
  if(rootOffset==0)
  {
    rootNum=bf.AddNewBlockToFile(name,1);
    rootOffset=1;
    char newData[MaxBlockNum];
    *(bool *)(newData+ValidPos)=true;
    *(bool *)(newData+TypePos)=true;
    *(short *)(newData+StorePos)=0;
    *(short *)(newData + WidthPos) = ((BlockMaxSize - BlockAttSpace) / (KeyLength + 4));
    *(int *)(newData+ParentPos)=0;
    bf.WriteData(rootNum, newData, 0, BlockMaxSize);
  }
  else 
  {
    rootNum=bf.FindBlock(name,rootOffset);
  }
  BpTree root(name, rootOffset, rootNum, type);
  int newRoot=root.InsertToLeaf(key.data(),val);
  bf.Unlock(rootNum);
  if(newRoot!=0) bf.WriteData(blockNum,(char*)&newRoot,RootPos,4);
  bf.Unlock(blockNum);
}

int IndexManager::Search(Index index, std::string key)
{
  std::string name=GetFileName(index);
  int blockNum = bf.FindBlock(name, 0);
  char *fileHead = bf.ReadBlockData(blockNum);
  short type = *(short *)(fileHead + TypePos);
  int rootOffset, rootNum;
  rootOffset = *(int *)(fileHead + RootPos);

  // char result[BlockMaxSize];

  if(rootOffset==0)
  {
    return 0;
  }
  else 
  {
    rootNum=bf.FindBlock(name,rootOffset);
    BpTree root(name,rootOffset,rootNum,type);
    int val=root.SearchKey(key.data());
    return val;
  }
}

void IndexManager::Delete(Index index, std::string key)
{
  std::string name=GetFileName(index);
  int blockNum = bf.FindBlock(name, 0);
  char *fileHead = bf.ReadBlockData(blockNum);
  short type = *(short *)(fileHead + TypePos);
  int rootOffset, rootNum;
  rootOffset = *(int *)(fileHead + RootPos);

  if(rootOffset!=0)
  {
    rootNum = bf.FindBlock(name, rootOffset);
    BpTree root(name, rootOffset, rootNum, type);
    int newRoot = root.DeleteKey(key.data());
    bf.Unlock(rootNum);
    if (newRoot != 0)
      bf.WriteData(blockNum, (char *)&newRoot, RootPos, 4);
  }
    bf.Unlock(blockNum);
}