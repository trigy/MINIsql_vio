#ifndef __RECORD_
#define __RECORD_

#include<iostream>
#include"BufferManager.h"
#include "Minisql.h"
#define MaxOffsetPos 0
#define LengthPos 4

#define ValidPos 0
#define NullPos 1
#define AttPos 5

extern BufferManager bf;
class RecordManager{
  public:
    int MaxOffset(Table table);
    int RecordLength(Table table);
    bool IsValid(Table table, int recordOffset);
    std::string GetFileName(Table table);
    void Create(Table table);
    void Insert(Table table, Record record);
    void Delete(Table table, int recordOffset);
    void ReadRecord(Table table, int recordOffset, Record& record);
    void DropRecordFile(Table table);
};



#endif