#include"IndexManager.h"

IndexManager IDM;

std::string IndexManager::GetFileName(Index index)
{
    return index.index_name + "-" + index.table_name + ".idx";
}

void IndexManager::CreateIndexHead(Index index, short type)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char newData[BlockMaxSize];
    *(int *)(newData + MaxOffsetPos_IM) = 0;
    *(short *)(newData + TypePos_IM) = type;
    *(int *)(newData + RootPos_IM) = 0;
    bf.WriteData(blockNum, newData, 0, BlockMaxSize);
    bf.Unlock(blockNum);
}

int IndexManager::Insert(Index index, char *key, int val)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    int maxOffset=*(int *)(fileHead + MaxOffsetPos_IM);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset, rootNum;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    if (rootOffset == 0)
    {
        rootNum = bf.FindBlock(name, 1);
        rootOffset = 1;
        bf.WriteData(blockNum, (char *)&rootOffset, RootPos_IM, 4);
        char newData[MaxBlockNum];
        *(bool *)(newData + ValidPos) = true;
        *(bool *)(newData + TypePos) = true;
        *(short *)(newData + WidthPos) = ((BlockMaxSize - BlockAttSpace) / (KeyLength + 4));
        *(short *)(newData + StorePos) = 0;
        *(int *)(newData + ParentPos) = 0;
        bf.WriteData(rootNum, newData, 0, BlockMaxSize);
    }
    else
    {
        rootNum = bf.FindBlock(name, rootOffset);
    }
    BpTree root(name, rootOffset, rootNum, type);
    int newRoot = root.InsertToLeaf(key, val);
    bf.Unlock(rootNum);
    if(newRoot==-1)
    {
        bf.Unlock(blockNum);
        return -1;
    }
    if (newRoot != 0)
        bf.WriteData(blockNum, (char *)&newRoot, RootPos_IM, 4);
    bf.Unlock(blockNum);
    return 0;
}

int IndexManager::Search(Index index, char *key, bool &exist) 
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset, rootNum;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    bf.Unlock(blockNum);

    if (rootOffset == 0)
    {
        exist=false;
        return 0;
    }
    else
    {
        rootNum = bf.FindBlock(name, rootOffset);
        BpTree root(name, rootOffset, rootNum, type);
        int val = root.SearchKey(key,exist);
        return val;
    }
}

int IndexManager::Delete(Index index, char *key)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset, rootNum;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    if (rootOffset != 0)
    {
        rootNum = bf.FindBlock(name, rootOffset);
        BpTree root(name, rootOffset, rootNum, type);
        int newRoot = root.DeleteKey(key);
        bf.Unlock(rootNum);
        if(newRoot==-1)
        {
            return -1;
        }
        if (newRoot != 0)
            bf.WriteData(blockNum, (char *)&newRoot, RootPos_IM, 4);
    }
    bf.Unlock(blockNum);
    return 0;
}

void IndexManager::SearchLarger(Index index, char* key, vector<int> &valList,bool isEqual)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    SearchLarger(name,rootOffset,key,type,valList,isEqual);
    bf.Unlock(blockNum);
}

void IndexManager::SearchLarger(std::string name, int offset, char *key, short type, vector<int> &valList, bool isEqual)
{
    int blockNum = bf.FindBlock(name, offset);
    BpTree bp(name, offset, blockNum, type);
    short indexNum;
    bool exist = bp.Search(key, indexNum);
    if (*(bool *)(bp.data + TypePos) == false)
    {
        int of = *(int *)(bp.data + BlockAttSpace + indexNum * (KeyLength + 4) + KeyLength);
        SearchLarger(name, of, key, type, valList, isEqual);
        for (short i = indexNum + 1; i < *(short *)(bp.data + StorePos); i++)
        {
            int cof = *(int *)(bp.data + BlockAttSpace + i * (KeyLength + 4) + KeyLength);
            int childNum = bf.FindBlock(name, cof);
            BpTree child(name, cof, childNum, type);
            short store = *(short *)(child.data + StorePos);
            GetInterval(name, child.data, 0, store - 1, type, valList);
            bf.Unlock(childNum);
        }
    }
    else
    {
        short store = *(short *)(bp.data + StorePos);
        if (isEqual && exist)
        {        
            GetInterval(name, bp.data, indexNum, store - 1, type, valList);
        }
        else
        {
            GetInterval(name, bp.data, indexNum+1, store - 1, type, valList);
        }
    }
    bf.Unlock(blockNum);
}

void IndexManager::SearchSmaller(Index index, char *key, vector<int> &valList, bool isEqual)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    SearchSmaller(name, rootOffset, key, type, valList, isEqual);
    bf.Unlock(blockNum);
}

void IndexManager::SearchSmaller(std::string name, int offset, char *key, short type, vector<int> &valList, bool isEqual)
{
    int blockNum = bf.FindBlock(name, offset);
    BpTree bp(name, offset, blockNum, type);
    short indexNum;
    bool exist = bp.Search(key, indexNum);
    if (*(bool *)(bp.data + TypePos) == false)
    {
        for (short i = 0; i < indexNum; i++)
        {
            int cof = *(int *)(bp.data + BlockAttSpace + i * (KeyLength + 4) + KeyLength);
            int childNum = bf.FindBlock(name, cof);
            BpTree child(name, cof, childNum, type);
            short store = *(short *)(child.data + StorePos);
            GetInterval(name, child.data, 0, store - 1, type, valList);
            bf.Unlock(childNum);
        }
        int of = *(int *)(bp.data + BlockAttSpace + indexNum * (KeyLength + 4) + KeyLength);
        SearchSmaller(name, of, key, type, valList, isEqual);
    }
    else
    {
        short store = *(short *)(bp.data + StorePos);
        if (isEqual && exist)
        {
            GetInterval(name, bp.data, 0, indexNum, type, valList);
        }
        else
        {
            GetInterval(name, bp.data, 0, indexNum-1, type, valList);
        }
    }
    bf.Unlock(blockNum);
}

void IndexManager::GetInterval(std::string name, char *data, short start, short end, short type, vector<int> &valList)
{
    for(short i=start;i<=end;i++)
    {
        if(*(bool*)(data+TypePos)==true)
        {
            valList.push_back(*(int*)(data+BlockAttSpace+i*(KeyLength+4)+KeyLength));
        }
        else 
        {
            int cof = *(int *)(data + BlockAttSpace + i * (KeyLength + 4) + KeyLength);
            int childNum = bf.FindBlock(name, cof);
            BpTree child(name, cof, childNum, type);
            short store = *(short *)(child.data + StorePos);
            GetInterval(name, child.data, 0, store - 1, type, valList);
            bf.Unlock(childNum);
        }
    }
}

void IndexManager::DeleteAll(Index index)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    int maxOffset=-1;
    bf.WriteData(blockNum, (char*)&maxOffset, MaxOffsetPos_IM, 4);
    bf.Unlock(blockNum);
}

void IndexManager::DropIndexFile(Index index)
{
    std::string name = GetFileName(index);
    bf.DropFile(name);
}
