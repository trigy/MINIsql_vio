#include"IndexManager.h"

IndexManager IDM;

std::string IndexManager::GetFileName(Index index)
{
    return index.index_name + "-" + index.table_name + ".idx";
}

void IndexManager::CreateIndexHead(Index index, short type)
{
    std::string name = GetFileName(index);
    int blockNum = bf.AddNewBlockToFile(name, 0);
    char newData[BlockMaxSize];
    *(int *)(newData) = -1;
    *(short *)(newData + TypePos_IM) = type;
    *(int *)(newData + RootPos_IM) = 0;
    bf.WriteData(blockNum, newData, 0, BlockMaxSize);
}

void IndexManager::Insert(Index index, char *key, int val)
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    int maxOffset=*(int *)(fileHead + MaxOffsetPos_IM);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset, rootNum;
    rootOffset = *(int *)(fileHead + RootPos_IM);
    // cout<<"rootOffset: "<<rootOffset<<endl;
    if (rootOffset == 0)
    {
        rootNum = bf.AddNewBlockToFile(name, 1);
        // cout<<rootNum<<endl;
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
    // cout << "rootOffset: " << rootOffset << endl;
    BpTree root(name, rootOffset, rootNum, type);
    int newRoot = root.InsertToLeaf(key, val);
    cout<<"newRoot: "<<newRoot<<endl;
    bf.Unlock(rootNum);
    if (newRoot != 0)
        bf.WriteData(blockNum, (char *)&newRoot, RootPos_IM, 4);
        maxOffset++;
    bf.WriteData(blockNum,(char*)&maxOffset,MaxOffsetPos_IM,4);
    bf.Unlock(blockNum);
}

int IndexManager::Search(Index index, char *key, bool &exist) 
{
    std::string name = GetFileName(index);
    int blockNum = bf.FindBlock(name, 0);
    char *fileHead = bf.ReadBlockData(blockNum);
    short type = *(short *)(fileHead + TypePos_IM);
    int rootOffset, rootNum;
    rootOffset = *(int *)(fileHead + RootPos_IM);

    // char result[BlockMaxSize];

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

void IndexManager::Delete(Index index, char *key)
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
        if (newRoot != 0)
            bf.WriteData(blockNum, (char *)&newRoot, RootPos_IM, 4);
    }
    bf.Unlock(blockNum);
}
