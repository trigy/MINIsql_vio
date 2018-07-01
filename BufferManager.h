#ifndef __BUFF_
#define __BUFF_

#include<iostream>
#include<fstream>
#include<list>
#include<string>
#define BlockMaxSize 4096
#define MaxBlockNum 256
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
    
    bool IsFull();
    void AdjustLRU(int index);
    void DropBlockLRU();
    void Lock(int index);
    int ReadBlockFromFile(std::string fileName, int offset);
  public:
    BufferManager();
    int FindBlock(std::string fileName, int offset);
    int FindFreeBlockFromFile(std::string fileName, int& offset);
    char* ReadBlockData(int index);
    void WriteData(int index, char* newData, int start, int length);
    void Unlock(int index);
    void WriteBackAllDirtyBlock();
    bool FileExist(std::string fileName);
    void DropFile(std::string fileName);
    // int AddNewBlockToFile(std::string fileName, int offset);
};

#endif