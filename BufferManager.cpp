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
    // std::fstream ifile(fileName, std::ios::in | std::ios::out | std::ios::binary);
    // int index;
    // char newData[BlockMaxSize];
    // ifile.seekg(0);
    // ifile.read(newData, BlockMaxSize);
    // std::cout<<"firstblock: "<<*(short*)newData<<std::endl;
    // ifile.close();

    std::fstream file(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary);
    std::cout<<"state: "<<file.good()<<std::endl;
    if(!file.good())
    {
        file.close();
        file.open(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        file.clear();
    }
    file.seekp(offset * BlockMaxSize);
    std::cout << "offset: " << (offset * BlockMaxSize) << std::endl;
    file.write(data, BlockMaxSize);
    file.flush();
    std::cout << "first: " << *(short *)data << std::endl;
    dirty = false;
    file.close();
    std::cout<<"writeback"<<std::endl;

    std::fstream ifile2(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary);
    int index2;
    char newData2[BlockMaxSize];
    ifile2.seekg(0);
    ifile2.read(newData2, BlockMaxSize);
    std::cout << "firstblock: " << *(short *)newData2 << std::endl;
    ifile2.close();
    std::cout<<std::endl;
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
    std::fstream file(FILEDIR + name);
    if (file.good())
    {
        file.close();
        return true;
    }
    else
    {
        file.close();
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
    while (block[*(--it)].IsPin())
        ;
    blankIndex = *it;
    if (block[*it].IsDirty())
    {
        block[*it].WriteBack();
    }
    // lruIndex.erase(it);
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
    file.close();
    lruIndex.push_front(index);
    Lock(index);
    return index;
}

int BufferManager::FindBlock(std::string fileName, int offset)
{
    for (int i = 0; i < store; i++)
    {
        if (block[i].Match(fileName, offset))
        {
            AdjustLRU(i);
            Lock(i);
            return i;
        }
    }
    return ReadBlockFromFile(fileName, offset);
}

int BufferManager::FindFreeBlockFromFile(std::string fileName, int &offset)
{
    std::fstream file(FILEDIR + fileName, std::ios::in | std::ios::out | std::ios::binary);
    bool valid;
    int maxOffset;
    file.seekg(0);
    file.read((char *)&maxOffset, 4);
    for (int i = 1; i <= maxOffset; i++)
    {
        file.seekg(i * BlockMaxSize);
        file.read((char *)&valid, 1);
        if (valid == false)
        {
            file.close();
            offset = i;
            return ReadBlockFromFile(fileName, i);
        }
    }
    file.close();
    offset = maxOffset++;
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
        index = store++;
    }
    bool exist = FileExist(fileName);
    if (!(exist & offset))
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
    std::cout<<"called, store = "<<store<<std::endl;
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
