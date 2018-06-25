#include<iostream>
#include"BufferManager.h"
#include "Minisql.h"
#define MaxOffsetPos 0
#define LengthPos 4

#define ValidPos 0
#define NullPos 1
#define AttPos 9

extern BufferManager bf;
class RecordManager{
  public:
    int MaxOffset(Table table);
    int RecordLength(Table table);
    // bool IsValid(Table table, int recordOffset);
    std::string GetFileName(Table table);
    void Create(Table table);
    void Insert(Table table, Record record);
    void Delete(Table table, int recordOffset);
    Record ReadRecord(Table table, int recordOffset);
};
std::string RecordManager::GetFileName(Table table)
{
  return table.table_name+".rec"; 
}

int RecordManager::MaxOffset(Table table)
{
  std::string name = GetFileName(table);
  int blockNum = bf.FindBlock(name, 0);
  char *data = bf.ReadBlockData(blockNum);
  int maxOffset = *(bool *)(data + MaxOffsetPos);
  bf.Unlock(blockNum);
  return maxOffset;
}

int RecordManager::RecordLength(Table table)
{
  std::string name = GetFileName(table);
  int blockNum = bf.FindBlock(name, 0);
  char *data = bf.ReadBlockData(blockNum);
  int lengthPerRecord = *(int *)(data + LengthPos);
  bf.Unlock(blockNum);
  return lengthPerRecord;
}

// bool RecordManager::IsValid(Table table, int recordOffset)
// {
//   int length=RecordLength(table);
//   std::string name = GetFileName(table);
//   int recordNum = bf.FindBlock(name, 1);
//   char* data=bf.ReadBlockData(recordNum);
//   bool valid=*(bool*)(data+recordOffset*length+ValidPos);
//   bf.Unlock(recordNum);
//   return valid;
// }

void RecordManager::Create(Table table)
{
  std::string name=GetFileName(table);
  int blockNum=bf.AddNewBlockToFile(name,0);
  int maxOffset=0;
  int lengthPerRecord=5+4*table.attr_num;
  for(int i=0;i<table.attr_num;i++)
  {
    lengthPerRecord+=(table.attrs[i].attr_type>0?table.attrs[i].attr_type:4);
  }
  bf.WriteData(blockNum,(char*)&maxOffset,MaxOffsetPos,4);
  bf.WriteData(blockNum,(char*)lengthPerRecord,LengthPos,4);
  bf.Unlock(blockNum);
  int recordNum=bf.AddNewBlockToFile(name,1);
  bf.Unlock(recordNum);
}

void RecordManager::Insert(Table table, Record record)
{
  std::string name = GetFileName(table);
  int blockNum=bf.FindBlock(name,0);
  char* data=bf.ReadBlockData(blockNum);
  int maxOffset= *(int *)(data + MaxOffsetPos);
  int lengthPerRecord = *(int *)(data + LengthPos);
  int recordPerBlock=BlockMaxSize/lengthPerRecord;
  int blockIndex=(maxOffset+1)/recordPerBlock+1;
  int recordOffset = (maxOffset + 1) % recordPerBlock;

  int recordNum=bf.FindBlock(name,blockIndex);
  char newData[BlockMaxSize];
  bool valid=true;
  char nullMap[4]={0};
  memcpy(newData+ValidPos,(char*)&valid,1);
  memcpy(newData+NullPos,nullMap,4);
  short recordPos=AttPos+4*table.attr_num;
  for(int i=0;i<table.attr_num;i++)
  {
    short type=table.attrs[i].attr_type;
    short length = type > 0 ? type : 4;
    memcpy(newData+AttPos+i*4,(char*)&recordPos,2);
    memcpy(newData+AttPos+i*4+2,(char*)&length,2);
    char* key=record.atts[i].data();
    memcpy(newData+recordPos,key,length);
    if(record.null[i])
    {
      *(newData+NullPos+i/4)|=bitMap[i%4];
    }
    recordPos+=length;
  }
  bf.WriteData(recordNum,newData,recordOffset*lengthPerRecord,lengthPerRecord);
  bf.Unlock(recordNum);
  maxOffset++;
  bf.WriteData(blockNum,(char*)&maxOffset,MaxOffsetPos,4);
  bf.Unlock(blockNum);
}

void RecordManager::Delete(Table table, int recordOffset)
{
  std::string name = GetFileName(table);
  int blockNum = bf.FindBlock(name, 0);
  char *data = bf.ReadBlockData(blockNum);
  int lengthPerRecord = *(int *)(data + LengthPos);
  int recordPerBlock = BlockMaxSize / lengthPerRecord;
  int blockIndex = (recordOffset) / recordPerBlock + 1;
  int recordOffsetInBlock = (recordOffset) % recordPerBlock;

  int recordNum=bf.FindBlock(name,blockIndex);
  bool valid=false;
  bf.WriteData(recordNum,(char*)&valid,lengthPerRecord*recordOffsetInBlock+ValidPos,1);
  bf.Unlock(recordNum);
}

Record RecordManager::ReadRecord(Table table, int recordOffset)
{
  std::string name = GetFileName(table);
  int blockNum = bf.FindBlock(name, 0);
  char *data = bf.ReadBlockData(blockNum);
  int lengthPerRecord = *(int *)(data + LengthPos);
  int recordPerBlock = BlockMaxSize / lengthPerRecord;
  int blockIndex = (recordOffset) / recordPerBlock + 1;
  int recordOffsetInBlock = (recordOffset) % recordPerBlock;

  int recordNum = bf.FindBlock(name, blockIndex);
  char* recordData=bf.ReadBlockData(recordNum);
  Record record;
  for(int i=0;i<table.attr_num;i++)
  {
    int pos = *(short *)(recordData + recordOffsetInBlock*lengthPerRecord+ AttPos + 4 * i);
    std::string att((recordData + recordOffsetInBlock * lengthPerRecord + pos));
    char *nullMap = recordData + recordOffsetInBlock * lengthPerRecord +NullPos;
    bool isNull=*(nullMap+i/4) & bitMap[i%4];
    record.null.push_back(isNull);
    record.atts.push_back(att);
  }
  bf.Unlock(recordNum);
  return record;
}

// int RecordManager::Search(Table table, int attIndex, std::string key)
// {
//   std::string name=GetFileName(table);
//   int blockNum=bf.FindBlock(name,0);
//   char *data = bf.ReadBlockData(blockNum);
//   int lengthPerRecord = *(int *)(data + LengthPos);
//   int maxOffset=*(int*)(data + MaxOffsetPos);
//   bf.Unlock(blockNum);

//   int recordNum = bf.FindBlock(name, 1);
//   int val;
//   char *recordData=bf.ReadBlockData(recordNum);
//   fot(int i=0;)
// }

