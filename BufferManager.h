#ifndef __BUFF_
#define __BUFF_

#include<iostream>
#include<fstream>
#include<list>
#define BlockMaxSize 4096
#define MaxBlockNum 1024
#define FILEDIR "dbfile\\"
// #define MaxLength 20
class BlockNode {
  private:
    bool valid;
    int offset;
    bool pin;
    bool dirty;
    char data[BlockMaxSize];
    std::string fileName;
  public:
    // BlockNode();
    // ~BlockNode();
    void Init(int o, char* d, std::string f);
    char* Data();
    bool IsValid();
    bool Match(std::string f, int o);
    bool Match(std::string f);
    void SetPin(bool p);
    bool IsPin();
    void SetDirty(bool d);
    bool IsDirty();
    void Drop();
    void WriteData(int start, char* newData, int length);
    void WriteBack();  
};

class BufferManager{
  private:
    BlockNode block[MaxBlockNum];
    std::list<int> lruIndex;
    int store;
    int blankIndex;
  public:
    BufferManager();
    // ~BufferManager();
    bool IsFull();
    bool FileExist(std::string fileName);
    int ReadBlockFromFile(std::string fileName, int offset);
    void AdjustLRU(int index);
    void DropBlockLRU();
    void WriteBackAllDirtyBlock();
    int FindBlock(std::string fileName, int offset);
    int FindFreeBlockFromFile(std::string fileName, int& offset);
    int AddNewBlockToFile(std::string fileName, int offset);
    char* ReadBlockData(int index);
    void Lock(int index);
    void Unlock(int index);
    void WriteData(int index, char* newData, int start, int length);
    void DropFile(std::string fileName);
};

#endif