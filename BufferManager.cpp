#include"BufferManager.h"

BufferManager bf;

void BlockNode::SetPin(bool p)
{
    pin = p;
}

bool BlockNode::IsPin()
{
    return pin;
}

void BlockNode::SetDirty(bool d)
{
    dirty = d;
}

bool BlockNode::IsDirty()
{
    return dirty;
}

void BlockNode::Drop()
{
    valid = false;
}

bool BlockNode::IsValid()
{
    return valid;
}

char *BlockNode::Data()
{
    return data;
}

void BlockNode::Init(int o, char *d, std::string f)
{
    valid = true;
    pin = false;
    dirty = false;
    offset = o;
    memcpy(data, d, BlockMaxSize);
    fileName = f;
}

bool BlockNode::Match(std::string f, int o)
{
    return fileName == f && offset == o;
}

bool BlockNode::Match(std::string f)
{
    return fileName == f;
}

void BlockNode::WriteData(int start, char *newData, int length)
{
    dirty = true;
    memcpy(data + start, newData, length);
}

void BlockNode::WriteBack()
{
    std::fstream file(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary);
    if(!file.good())
    {
        file.close();
        file.open(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        file.clear();
    }
    file.seekp(offset * BlockMaxSize);
    file.write(data, BlockMaxSize);
    file.flush();
    dirty = false;
    file.close();
}

BufferManager::BufferManager()
{
    store = 0;
    blankIndex = 0;
}

bool BufferManager::IsFull()
{
    if (store == MaxBlockNum)
        return true;
    else
        return false;
}

bool BufferManager::FileExist(std::string name)
{
    std::fstream file(FILEDIR + name, std::ios::in | std::ios::out | std::ios::binary);
    if (file.good())
    {
        file.close();
        return true;
    }
    else
    {
        file.close();
        for(int i=0;i<store;i++)
        {
            if (block[i].Match(name)&&block[i].IsValid())
            {
                // std::cout<<i<<name<<std::endl;
                return true;
            }
        }
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
    std::list<int>::iterator it = lruIndex.end();
    while (block[*(--it)].IsPin());
    blankIndex = *it;
    if (block[*it].IsDirty())
    {
        block[*it].WriteBack();
    }
}

int BufferManager::ReadBlockFromFile(std::string fileName, int offset)
{
    std::fstream file(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary);
    int index;
    char newData[BlockMaxSize];
    file.seekg(offset * BlockMaxSize);
    file.read(newData, BlockMaxSize);
    if (IsFull() == true)
    {
        DropBlockLRU();
        index = blankIndex;
    }
    else
    {
        index = store++;
    }
    block[index].Init(offset, newData, fileName);
    // std::cout<<index<<std::endl;
    file.close();
    AdjustLRU(index);
    Lock(index);
    return index;
}

int BufferManager::FindBlock(std::string fileName, int offset)
{
    for (int i = 0; i < store; i++)
    {
        if (block[i].Match(fileName, offset)&&block[i].IsValid())
        {
            // std::cout<<i<<std::endl;
            AdjustLRU(i);
            Lock(i);
            return i;
        }
    }
    return ReadBlockFromFile(fileName, offset);
}

int BufferManager::FindFreeBlockFromFile(std::string fileName, int &offset)
{
    int blockNum=FindBlock(fileName,0);
    char* data=ReadBlockData(blockNum);
    (*(int*)data)++;
    offset = (*(int *)data)+1;
    // std::cout<<"offset: "<<offset<<std::endl;
    Unlock(blockNum);
    return FindBlock(fileName, offset);
}

char *BufferManager::ReadBlockData(int index)
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
    block[index].WriteData(start, newData, length);
}

void BufferManager::WriteBackAllDirtyBlock()
{
    for (int i = 0; i < store; i++)
    {
        if (block[i].IsValid() && block[i].IsDirty())
        {
            block[i].WriteBack();
        }
    }
}

void BufferManager::DropFile(std::string fileName)
{
    for (int i = 0; i < store; i++)
    {
        if (block[i].Match(fileName))
        {
            block[i].Drop();
        }
    }
    remove((FILEDIR+fileName).data());
}
