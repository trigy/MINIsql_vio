#include "BpTree.h"

extern BufferManager bf;

int cmp(char *data, KEY key, short type, short i)
{
    if (type == 0) //int
    {
        int a = *(int *)(data + i * (KeyLength + 4));
        int b = *(int *)key;
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
    else if (type == -1) //float
    {
        float a = *(float *)(data + i * (KeyLength + 4));
        float b = *(float *)key;
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
    else
    {
        KEY a = (data + i * (KeyLength + 4));
        KEY b = key;
        for (short j = 0; j < type; j++)
        {
            if (*(a + j) > *(b + j))
                return 1;
            else if (*(a + j) < *(b + j))
                return -1;
            else
               continue;;
        }
        return 0;
    }
}

BpTree::BpTree(std::string name, int offset, int blockNum, short type) : name(name), offset(offset), blockNum(blockNum), type(type)
{
    data = bf.ReadBlockData(blockNum);
}

int BpTree::BlockNum()
{
    return blockNum;
}

int BpTree::Offset()
{
    return offset;
}

int BpTree::InsertToLeaf(KEY key, int val)
{
    if (*(bool *)(data + TypePos) == true)
    {
        return InsertInThisNode(key, val);
    }
    else
    {
        short index = 0;
        Search(key, index);
        int childOffset = *(int *)(data + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
        int childNum = bf.FindBlock(name, childOffset);
        BpTree child(name, childOffset, childNum, type);
        bf.Unlock(blockNum);
        return child.InsertToLeaf(key, val);
    }
}

int BpTree::Split(short index)
{
    int brotherOffset;
    int brotherNum = bf.FindFreeBlockFromFile(name, brotherOffset);
    BpTree bro(name, brotherOffset, brotherNum, type);
    char broData[BlockMaxSize];
    *(bool*)(broData+ValidPos)=true;
    *(bool*)(broData+TypePos)=*(bool*)(data+TypePos);
    *(short *)(broData + WidthPos) = *(short *)(data + WidthPos);
    *(short *)(broData + ParentPos) = *(short *)(data + ParentPos);
    if (index < 2)
        index == 2;
    else if (index == (*(short *)(data + StorePos)) - 1)
        index--;
    memcpy(broData + BlockAttSpace, data + BlockAttSpace + index * (KeyLength + 4), ((*(short *)(data + StorePos) - index) * (KeyLength + 4)));
    *(short *)(broData + StorePos)=*(short*)(data+StorePos);
    *(short *)(broData + StorePos) -= index;
    bf.WriteData(blockNum, (char *)&index, StorePos, 2);
    bf.WriteData(brotherNum,broData,0,BlockMaxSize);
    if (*(int *)(data + ParentPos) == 0)
    {
        int rootOffset;
        int rootNum = bf.FindFreeBlockFromFile(name, rootOffset);
        BpTree root(name, rootOffset, rootNum, type);
        char *rootData = bf.ReadBlockData(rootNum);

        bool newValid = true, newType = false;
        short newWidth = *(short *)(data + WidthPos), newStore = 0;
        bf.WriteData(rootNum, (char *)&newValid, ValidPos, 1);
        bf.WriteData(rootNum, (char *)&newType, TypePos, 1);
        bf.WriteData(rootNum, (char *)&newWidth, WidthPos, 2);
        bf.WriteData(rootNum, (char *)&newStore, StorePos, 2);

        bf.WriteData(blockNum, (char *)&rootOffset, ParentPos, 4);
        bf.WriteData(brotherNum, (char *)&rootOffset, ParentPos, 4);

        bf.Unlock(blockNum);
        bf.Unlock(brotherNum);
        root.InsertInThisNode(data + BlockAttSpace, offset);
        root.InsertInThisNode(broData + BlockAttSpace, brotherOffset);
        bf.Unlock(rootNum);
        return rootOffset;
    }

    else
    {
        int parentOffset = *(int *)(data + ParentPos);
        int parentNum = bf.FindBlock(name, parentOffset);
        BpTree parent(name, parentOffset, parentNum, type);
        parent.InsertInThisNode(broData + BlockAttSpace, brotherOffset);
        bf.WriteData(brotherNum, (char *)&parentOffset, ParentPos, 4);
        bf.Unlock(brotherNum);
        char *parentData = bf.ReadBlockData(parentNum);
        if (*(short *)(parentData + StorePos) > *(short *)(parentData + WidthPos))
        {
            short childIndex;
            parent.Search(broData + BlockAttSpace, childIndex);
            return parent.Split(childIndex);
        }
        return 0;
    }
}

int BpTree::InsertInThisNode(KEY key, int val)
{
    short index;
    bool exist = Search(key, index);
    if (*(bool *)(data + TypePos) && exist)
    {
        return -1;
    }
    index++;
    char newData[BlockMaxSize];

    memcpy(newData, data, BlockAttSpace + index * (KeyLength + 4));
    memcpy(newData + BlockAttSpace + (index + 1) * (KeyLength + 4), data + BlockAttSpace + index * (KeyLength + 4), ((*(short *)(data + StorePos)) - index) * (KeyLength + 4));
    (*(short *)(newData + StorePos))++;
    memcpy(newData + BlockAttSpace + index * (KeyLength + 4), key, KeyLength);
    * (int *)(newData + BlockAttSpace + index * (KeyLength + 4) + KeyLength) = val;

    bf.WriteData(blockNum, newData, 0, BlockMaxSize);

    if (*(short *)(data + StorePos) > *(short *)(data + WidthPos))
    {
        return Split(index);
    }
    bf.Unlock(blockNum);
    return 0;
}

bool BpTree::Search(KEY key, short &index)
{
    short store = *(short *)(data + StorePos);
    if (store < tipNum) //when the num of key is lower than setting number, we search one by one
    {
        for (short i = 0; i < store; i++)
        {
            int result = cmp(data + BlockAttSpace, key, type, i);
            if (result == 0)
            {
                index = i;
                return true;
            }
            else if (result > 0)
            {
                index = i - 1;
                return false;
            }
        }
        index = store - 1;
        return false;
    }
    else
    {
        short left = 0;
        short right = store - 1;
        while (left < right)
        {
            short mid = (left + right) / 2;
            int result = cmp(data + BlockAttSpace, key, type, mid);
            if (result < 0)
            {
                right = mid - 1;
            }
            else if (result > 0)
            {
                left = mid + 1;
            }
            else
            {
                index = mid;
                return true;
            }
        }
        index = right;
        return false;
    }
}

int BpTree::SearchKey(KEY key, bool &exist)
{
    short index;
    exist = Search(key, index);
    if (*(bool *)(data + TypePos) == true)
    {
        int val = *(int *)(data + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
        bf.Unlock(blockNum);
        return val;
    }
    else
    {
        int childOffset = *(int *)(data + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
        int childNum = bf.FindBlock(name, childOffset);
        BpTree child(name, childOffset, childNum, type);
        bf.Unlock(blockNum);
        return child.SearchKey(key,exist);
    }
}

int BpTree::DeleteKey(KEY key)
{
    short index;
    bool exist = Search(key, index);
    if (*(bool *)(data + TypePos) == true)
    {
        if (!exist)
        {
            return -1;
        }
        char newData[BlockMaxSize];
        memcpy(newData, data, BlockAttSpace + index * (KeyLength + 4));
        memcpy(newData + BlockAttSpace + index * (KeyLength + 4), data + BlockAttSpace + (index + 1) * (KeyLength + 4), (*(short *)(data + StorePos) - index - 1) * (KeyLength + 4));
        *(short *)(newData + StorePos) = (*(short *)(newData + StorePos)) - 1;
        bf.WriteData(blockNum, newData, 0, BlockMaxSize);
        if (*(short *)(newData + StorePos) == 1)
        {
            return Reform();
        }
        bf.Unlock(blockNum);
        return 0;
    }
    else
    {
        short index = 0;
        Search(key, index);
        int childOffset = *(int *)(data + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
        int childNum = bf.FindBlock(name, childOffset);
        BpTree child(name, childOffset, childNum, type);
        bf.Unlock(blockNum);
        return child.DeleteKey(key);
    }
}

int BpTree::Reform()
{
    if(*(bool*)(data+TypePos)==true)
    {
        return 0;
    }
    short index;
    if (*(int *)(data + ParentPos) == 0)
    {
        int childOffset = *(int *)(data + BlockAttSpace + KeyLength);
        bool newValid = false;
        bf.WriteData(blockNum, (char *)newValid, ValidPos, 1);
        bf.Unlock(blockNum);
        return childOffset;           
    }
    int parentOffset = *(int *)(data + ParentPos);
    int parentNum = bf.FindBlock(name, parentOffset);
    BpTree parent(name, parentOffset, parentNum, type);
    parent.Search(data + BlockAttSpace, index);
    char *parentData = bf.ReadBlockData(parentNum);
    short blockIndex=index;
    if (index == (*(short *)(parentData + StorePos)) - 1)
        index--;
    else
        index++;
    int brotherOffset = *(int *)(parentData + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
    int brotherNum = bf.FindBlock(name, brotherOffset);
    BpTree brother(name, brotherOffset, brotherNum, type);
    char *brotherData = bf.ReadBlockData(brotherNum);
    brother.InsertInThisNode(data + BlockAttSpace, *(int *)(data + BlockAttSpace + KeyLength));

    memcpy(parentData+BlockAttSpace+index*(KeyLength+4),brotherData+BlockAttSpace,KeyLength);

    char npd[BlockMaxSize];
    memcpy(npd, parentData, BlockAttSpace + blockIndex * (KeyLength + 4));
    memcpy(npd + BlockAttSpace + blockIndex * (KeyLength + 4), parentData + BlockAttSpace + (blockIndex + 1) * (KeyLength + 4), (*(short *)(parentData + StorePos) - blockIndex - 1) * (KeyLength + 4));
    (*(short*)(npd+StorePos))--;
    bf.WriteData(parentNum,npd,0,BlockMaxSize);
    bool newValid = false;
    bf.WriteData(blockNum, (char *)&newValid, ValidPos, 1);
    bf.Unlock(blockNum);
    if (*(short *)(parentData + StorePos) == 1)
    {
        bf.Unlock(brotherNum);
        return parent.Reform();
    }
    bf.Unlock(parentNum);
    bf.Unlock(brotherNum);
    return 0;
}
