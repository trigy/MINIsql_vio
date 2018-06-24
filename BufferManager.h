#include<iostream>
#include<fstream>
#include<list>
#define BlockMaxSize 4096
#define MaxBlockNum 1024
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
    BlockNode();
    ~BlockNode();
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

void BlockNode::SetPin(bool p)
{
  pin=p;
}

bool BlockNode::IsPin()
{
  return pin;
}

void BlockNode::SetDirty(bool d)
{
  dirty=d;
}

bool BlockNode::IsDirty()
{
  return dirty;
}

void BlockNode::Drop()
{
  valid=false;
}

bool BlockNode::IsValid()
{
  return valid;
}

char* BlockNode::Data()
{
  return data;
}

void BlockNode::Init(int o, char* d, std::string f)
{
  valid=true;
  pin=false;
  dirty=false;
  offset=o;
  memcpy(data,d,BlockMaxSize);
  fileName=f;
}

bool BlockNode::Match(std::string f, int o)
{
  return fileName==f&&offset==o;
}

bool BlockNode::Match(std::string f)
{
  return fileName==f;
}

void BlockNode::WriteData(int start, char *newData, int length)
{
  dirty=true;
  memcpy(data+start,newData,length);
}

void BlockNode::WriteBack()
{
  std::ofstream file(fileName);
  file.seekp(offset*BlockMaxSize);
  file.write(data,BlockMaxSize);
  dirty=false;
  file.close();
}

class BufferManager{
  private:
    BlockNode block[MaxBlockNum];
    std::list<int> lruIndex;
    int store;
    int blankIndex;
  public:
    BufferManager();
    ~BufferManager();
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

BufferManager::BufferManager()
{
  store=0;
  blankIndex=0;
}

bool BufferManager::IsFull()
{
  if(store==MaxBlockNum) return true;
  else return false;
}

bool BufferManager::FileExist(std::string name)
{
  std::ifstream file(name);
  if(file)
  {
    return true;
  }
  else 
  {
    return false;
  }
}

void BufferManager::AdjustLRU(int index)
{
  lruIndex.remove(index);
  lruIndex.push_front(index);
}

void BufferManager::DropBlockLRU()
{
  std::list<int>::iterator it=lruIndex.end();
  while(block[*(--it)].IsPin());
  blankIndex=*it;
  if(block[*it].IsDirty())
  {
    block[*it].WriteBack();
  }
  // lruIndex.erase(it);
}

int BufferManager::ReadBlockFromFile(std::string fileName, int offset)
{
  std::ifstream file(fileName.data());
  int index;
  char newData[BlockMaxSize];
  file.seekg(offset*BlockMaxSize);
  file.read(newData,BlockMaxSize);
  if(IsFull()==true)
  {
    DropBlockLRU();
    index=blankIndex;
  } 
  else
  {
    index=store++;
  }
  block[index].Init(offset,newData,fileName);
  file.close();
  lruIndex.push_front(index);
  Lock(index);
  return index;
}

int BufferManager::FindBlock(std::string fileName, int offset)
{
  for(int i=0;i<store;i++)
  {
    if(block[i].Match(fileName,offset)) 
    {
      AdjustLRU(i);
      Lock(i);
      return i;
    }
  }
  return ReadBlockFromFile(fileName,offset);
}

int BufferManager::FindFreeBlockFromFile(std::string fileName, int& offset)
{
  std::ifstream file(fileName.data());
  bool valid;
  int maxOffset;
  file.seekg(6);
  file.read((char *)&maxOffset, 4);
  for (int i = 1; i <= maxOffset; i++)
  {
    file.seekg(i*BlockMaxSize);
    file.read((char*)&valid,1);
    if(valid==false)
    {
      file.close();
      offset=i;
      return ReadBlockFromFile(fileName,i);
    }
  }
  file.close();
  offset=maxOffset++;
  return AddNewBlockToFile(fileName, offset);
}

int BufferManager::AddNewBlockToFile(std::string fileName, int offset)
{
  int index;
  char newData[BlockMaxSize];
  if (IsFull() == true)
  {
    DropBlockLRU();
    index = blankIndex;
  }
  else
  {
    index = blankIndex++;
  }
  bool exist=FileExist(fileName);
  if(!(exist&offset))
  {
    block[index].Init(offset, newData, fileName);
    block[index].SetDirty(1);
    lruIndex.push_front(index);
    Lock(index);
    return index;
  }
  else
  {
    return -1;
  }
}

char* BufferManager::ReadBlockData(int index)
{
  return block[index].Data();
}

void BufferManager::Lock(int index)
{
  block[index].SetPin(1);
}

void BufferManager::Unlock(int index)
{
  block[index].SetPin(0);
}

void BufferManager::WriteData(int index, char *newData, int start, int length)
{
  block[index].WriteData(start,newData,length);
}

void BufferManager::WriteBackAllDirtyBlock()
{
  for(int i=0;i<store;i++)
  {
    if(block[i].IsValid()&&block[i].IsDirty())
    {
      block[i].WriteBack();
    }
  }
}

void BufferManager::DropFile(std::string fileName)
{
  for(int i=0;i<store;i++)
  {
    if(block[i].Match(fileName))
    {
      block[i].Drop();
    }
  }
  remove(fileName.data());
}
