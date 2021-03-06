#include"CatalogManager.h"

CatalogManager CTM;
extern BufferManager bf;
std::string CatalogManager::GetFileName(std::string tableName)
{
    // cout<<tableName<<".tbl"<<endl;
    return tableName + ".tbl";
}

int CatalogManager::CreateTable(Table table)
{
    std::string name = GetFileName(table.table_name);
    int blockNum = bf.FindBlock(name, 0);
    if (blockNum == -1)
    {
        return -1;
    }
    char newData[BlockMaxSize];
    char unique[4]={0};
    *(short *)(newData + AttNumPos) = table.attr_num;
    bool primaryDef = false;
    short dataPos = TableAttPos + 6 * table.attr_num;
    for (int i = 0; i < table.attr_num; i++)
    {
        if (table.attrs[i].attr_key_type == PRIMARY)
        {
            if (!primaryDef)
            {
                *(short *)(newData + PrimaryNumPos) = i;
            }
            else
            {
                return -2;
            }
        }
        else if (table.attrs[i].attr_key_type == UNIQUE)
        {
            unique[i/8] |= bitMap[i % 8];
        }
        short length = table.attrs[i].attr_name.size();
        short type = table.attrs[i].attr_type;
        const char *attName = table.attrs[i].attr_name.data();
        memcpy(newData + TableAttPos + i * 6, (char *)&dataPos, 2);
        memcpy(newData + TableAttPos + i * 6 + 2, (char *)&length, 2);
        memcpy(newData + TableAttPos + i * 6 + 4, (char *)&type, 2);
        memcpy(newData + dataPos, attName, length);
        dataPos += length;
    }
    memcpy(newData+UniquePos,unique,4);
    bf.WriteData(blockNum, newData, 0, BlockMaxSize);
    IndexInit(name);
    bf.Unlock(blockNum);
    return 0;
}

bool CatalogManager::IndexInit(std::string name)
{
    int blockNum = bf.FindBlock(name, 1);
    short indexNum = 0;
    bf.WriteData(blockNum, (char *)&indexNum, IndexNumPos, 2);
    bf.Unlock(blockNum);
    return 0;
}

void CatalogManager::CreateIndex(Index index)
{
    std::string name = GetFileName(index.table_name);
    int blockNum = bf.FindBlock(name, 1);
    char *data = bf.ReadBlockData(blockNum);
    short indexNum = *(short *)(data + IndexNumPos);
    const char *indexName = index.index_name.data();
    const char *attName = index.attr_name.data();
    bf.WriteData(blockNum, (char *)indexName, IndexAttPos + indexNum * 2 * WordMaxLength, WordMaxLength);
    bf.WriteData(blockNum, (char *)attName, IndexAttPos + indexNum * 2 * WordMaxLength + WordMaxLength, WordMaxLength);
    indexNum++;
    bf.WriteData(blockNum, (char *)&indexNum, IndexNumPos, 2);
    bf.Unlock(blockNum);
}

bool CatalogManager::TableExist(std::string name)
{
    std::string fileName = GetFileName(name);
    return bf.FileExist(fileName);
}

Table CatalogManager::ReadTable(std::string name)
{
    std::string fileName = GetFileName(name);
    int blockNum = bf.FindBlock(fileName, 0);
    char *data = bf.ReadBlockData(blockNum);
    short attNum = *(short *)(data + AttNumPos);
    short primaryNum = *(short *)(data + PrimaryNumPos);
    char *uniqueMap = data + UniquePos;
    Table table;
    table.table_name = name;
    table.attr_num = attNum;
    for (short i = 0; i < attNum; i++)
    {
        short pos = *(short *)(data + TableAttPos + i * 6);
        short length = *(short *)(data + TableAttPos + i * 6 + 2);
        short type = *(short *)(data + TableAttPos + i * 6 + 4);
        char attName[20];
        memcpy(attName, data + pos, length);
        attName[length]='\0';
        std::string attNameS(attName);
        int keyType;
        if (primaryNum == i)
            keyType = PRIMARY;
        else if (*(uniqueMap + i / 8) & bitMap[i % 8])
            keyType = UNIQUE;
        else
            keyType = OTHER;
        Attribute attr(attNameS, type, keyType, i);
        table.attrs.push_back(attr);
    }
    bf.Unlock(blockNum);
    return table;
}

int CatalogManager::IndexOffset(std::string tableName, std::string indexName)
{
    std::string fileName = GetFileName(tableName);
    int blockNum = bf.FindBlock(fileName, 1);
    char *data = bf.ReadBlockData(blockNum);
    for (short i = 0; i < *(short *)(data + IndexNumPos); i++)
    {
        std::string str(data + IndexAttPos + WordMaxLength * 2 * i);
        if (str == indexName)
        {
            bf.Unlock(blockNum);
            return i;
        }
    }
    bf.Unlock(blockNum);
    return -1;
}

int CatalogManager::IndexOffset2(std::string tableName, std::string attrName)
{
    std::string fileName = GetFileName(tableName);
    int blockNum = bf.FindBlock(fileName, 1);
    char *data = bf.ReadBlockData(blockNum);
    for (short i = 0; i < *(short *)(data + IndexNumPos); i++)
    {
        std::string str(data + IndexAttPos + WordMaxLength * 2 * i + WordMaxLength);
        if (str == attrName)
        {
            bf.Unlock(blockNum);
            return i;
        }
    }
    bf.Unlock(blockNum);
    return -1;
}

Index CatalogManager::ReadIndex(std::string tableName, std::string attrName, int indexOffset)
{
    std::string fileName = GetFileName(tableName);
    int blockNum = bf.FindBlock(fileName, 1);
    char *data = bf.ReadBlockData(blockNum);
    std::string indexName(data + IndexAttPos + WordMaxLength * 2 * indexOffset);
    Index index(indexName, tableName, attrName);
    bf.Unlock(blockNum);
    return index;
}

void CatalogManager::DropTable(Table table)
{
    std::string name = GetFileName(table.table_name);
    bf.DropFile(name);
}

void CatalogManager::DropIndex(std::string tableName, int indexOffset)
{
    std::string name = GetFileName(tableName);
    int blockNum = bf.FindBlock(name, 1);
    char *data = bf.ReadBlockData(blockNum);
    char newData[BlockMaxSize];
    memcpy(newData, data, BlockMaxSize);
    (*(short *)(newData + IndexNumPos))--;
    memcpy(newData + IndexAttPos + indexOffset * 2 * WordMaxLength, data + IndexAttPos + (indexOffset + 1) * 2 * WordMaxLength, BlockMaxSize - (IndexAttPos + indexOffset * 2 * WordMaxLength));
    bf.WriteData(blockNum, newData, 0, MaxBlockNum);
    bf.Unlock(blockNum);
}

int CatalogManager::GetIndexList(Table table, vector<Index> &indexList)
{
    std::string name = GetFileName(table.table_name);
    int blockNum = bf.FindBlock(name, 1);
    char *data = bf.ReadBlockData(blockNum);
    short indexNum=*(short*)(data+IndexNumPos);
    if(indexNum==0)
    {
        bf.Unlock(blockNum);
        return 0;
    }
    for(int i=0;i<indexNum;i++)
    {
        Index index;
        index.table_name=table.table_name;
        index.attr_name=string(data+IndexAttPos+WordMaxLength*i*2+WordMaxLength);
        index.index_name=string(data+IndexAttPos+WordMaxLength*i*2);
        indexList.push_back(index);
    }
    bf.Unlock(blockNum);
    return 1;
}
