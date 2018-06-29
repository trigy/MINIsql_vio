#include "BpTree.h"

extern BufferManager bf;

int cmp(char *data, KEY key, short type, short i)
{
    if (type == 0) //int
    {
        int a = *(int *)(data + i * (KeyLength + 4));
        int b = *(int *)key;
        // std::cout<<"a: "<<a<<"\tb: "<<b<<std::endl;
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
                return 0;
        }
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
        // std::cout<<"insert in this node"<<std::endl;
        return InsertInThisNode(key, val);
    }
    else
    {
        short index = 0;
        Search(key, index);
        // std::cout<<"childIndex: "<<index<<std::endl;
        int childOffset = *(int *)(data + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
        // std::cout<<"childOffset: "<<childOffset<<std::endl;
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
    // std::cout<<"broNum: "<<brotherNum<<std::endl;
    BpTree bro(name, brotherOffset, brotherNum, type);
    // char *broData = bf.ReadBlockData(brotherNum);
    char broData[BlockMaxSize];
    *(bool*)(broData+ValidPos)=true;
    *(bool*)(broData+TypePos)=*(bool*)(data+TypePos);
    *(short *)(broData + WidthPos) = *(short *)(data + WidthPos);
    *(short *)(broData + ParentPos) = *(short *)(data + ParentPos);
    // bf.WriteData(brotherNum, broData, 0, BlockAttSpace);
    // memcpy(broData,data,BlockAttSpace);
    if (index < 2)
        index == 2;
    else if (index == (*(short *)(data + StorePos)) - 1)
        index--;
    // std::cout<<"index: "<<index<<std::endl;
    // std::cout << "store: " << (*(int *)(data + StorePos)) << std::endl;
    // bf.WriteData(brotherNum, broData + BlockAttSpace, splitIndex * (KeyLength + 4), *(int *)(data + StorePos) - splitIndex * (KeyLength + 4));
    memcpy(broData + BlockAttSpace, data + BlockAttSpace + index * (KeyLength + 4), ((*(short *)(data + StorePos) - index) * (KeyLength + 4)));
    // std::cout << "blockNum:" << blockNum << std::endl;
    *(short *)(broData + StorePos)=*(short*)(data+StorePos);
    *(short *)(broData + StorePos) -= index;
    bf.WriteData(blockNum, (char *)&index, StorePos, 2);
    // *(int *)(data + StorePos)=index;
    // std::cout<<"brostore: "<<broStore<<std::endl;
    bf.WriteData(brotherNum,broData,0,BlockMaxSize);
    if (*(int *)(data + ParentPos) == 0)
    {
        // std::cout<<"there"<<std::endl;
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

        // *(bool*)(rootData+ValidPos)=true;
        // *(bool*)(rootData+TypePos)=false;
        // *(short*)(rootData+WidthPos)=*(short*)(data+WidthPos);
        // *(short*)(rootData+StorePos)=0;
        bf.WriteData(blockNum, (char *)&rootOffset, ParentPos, 4);
        bf.WriteData(brotherNum, (char *)&rootOffset, ParentPos, 4);

        // *(int*)(data+ParentPos)=rootOffset;
        // *(int*)(broData+ParentPos)=rootOffset;
        bf.Unlock(blockNum);
        bf.Unlock(brotherNum);
        root.InsertInThisNode(data + BlockAttSpace, offset);
        root.InsertInThisNode(broData + BlockAttSpace, brotherOffset);
        bf.Unlock(rootNum);
        return rootOffset;
    }

    else
    {
        // std::cout << "there?" << std::endl;
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
    if (*(bool *)(data + TypePos) == true && exist)
    {
        return -1;
    }
    index++;
    // std::cout<<"index: "<<index<<std::endl;
    char newData[BlockMaxSize];

    memcpy(newData, data, BlockAttSpace + index * (KeyLength + 4));
    memcpy(newData + BlockAttSpace + (index + 1) * (KeyLength + 4), data + BlockAttSpace + index * (KeyLength + 4), ((*(short *)(data + StorePos)) - index) * (KeyLength + 4));
    (*(short *)(newData + StorePos))++;
    // std::cout << "store: " << (*(short *)(newData + StorePos)) << std::endl;
    // std::cout << "width: " << (*(short *)(newData + WidthPos)) << std::endl;
    memcpy(newData + BlockAttSpace + index * (KeyLength + 4), key, KeyLength);
    // std::cout << "offset: " << BlockAttSpace + index * (KeyLength + 4)<<std::endl;
    // (char *)(newData + BlockAttSpace + index * (KeyLength + 4)) = key;
    * (int *)(newData + BlockAttSpace + index * (KeyLength + 4) + KeyLength) = val;

    bf.WriteData(blockNum, newData, 0, BlockMaxSize);

    if (*(short *)(data + StorePos) > *(short *)(data + WidthPos))
    {
        // std::cout<<"split"<<std::endl;
        return Split(index);
    }
    bf.Unlock(blockNum);
    return 0;
}

bool BpTree::Search(KEY key, short &index)
{
    short store = *(short *)(data + StorePos);
    // std::cout<<"store: "<<store<<std::endl;
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
            //error
        }
        char *newData;
        memcpy(newData, data, BlockAttSpace + index * (KeyLength + 4));
        memcpy(newData + BlockAttSpace + index * (KeyLength + 4), data + BlockAttSpace + (index + 1) * (KeyLength + 4), (*(int *)(data + StorePos) - index - 1) * (KeyLength + 4));
        *(short *)(newData + StorePos) = *(short *)(newData + StorePos) - 1;
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
    short index;
    if (*(int *)(data + ParentPos) == 0)
    {
        int childOffset = *(int *)(data + BlockAttSpace + KeyLength);
        int childNum = bf.FindBlock(name, childOffset);
        bf.Unlock(blockNum);
        char *childData = bf.ReadBlockData(childNum);
        bool newValid = false;
        bf.WriteData(childNum, (char *)newValid, ValidPos, 1);
        // *(bool *)childData = false;
        bf.Unlock(childNum);
        return childOffset;
    }
    int parentOffset = *(int *)(data + ParentPos);
    int parentNum = bf.FindBlock(name, parentOffset);
    BpTree parent(name, parentOffset, parentNum, type);
    parent.Search(*(KEY *)(data + BlockAttSpace), index);
    char *parentData = bf.ReadBlockData(parentNum);

    if (index == (*(short *)(parentData + StorePos)) - 1)
        index--;
    else
        index++;
    int brotherOffset = *(int *)(parentData + BlockAttSpace + index * (KeyLength + 4) + KeyLength);
    int brotherNum = bf.FindBlock(name, brotherOffset);
    BpTree brother(name, brotherOffset, brotherNum, type);
    char *brotherData = bf.ReadBlockData(brotherNum);

    brother.InsertInThisNode(brotherData + BlockAttSpace, *(int *)(brotherData + BlockAttSpace + KeyLength));
    bool newValid = false;
    bf.WriteData(blockNum, (char *)newValid, ValidPos, 1);
    // *(bool*)(data+ValidPos)=false;
    bf.Unlock(blockNum);
    if (*(short *)(brotherData + StorePos) > 3)
    {
        brother.Split(*(short *)(brotherData + StorePos) / 2);
    }
    if (*(short *)(brotherData + StorePos) == 1)
    {
        bf.Unlock(brotherNum);
        return parent.Reform();
    }
    bf.Unlock(parentNum);
    bf.Unlock(brotherNum);
    return 0;
}
