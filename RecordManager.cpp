#include"RecordManager.h"

RecordManager RCM;

std::string RecordManager::GetFileName(Table table)
{
    return table.table_name + ".rec";
}

int RecordManager::MaxOffset(Table table)
{
    std::string name = GetFileName(table);
    int blockNum = bf.FindBlock(name, 0);
    char *data = bf.ReadBlockData(blockNum);
    int maxOffset = *(int *)(data + MaxOffsetPos);
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

bool RecordManager::IsValid(Table table, int recordOffset)
{
    std::string name = GetFileName(table);
    int blockNum = bf.FindBlock(name, 0);
    char *data = bf.ReadBlockData(blockNum);
    int lengthPerRecord = *(int *)(data + LengthPos);
    int recordPerBlock = BlockMaxSize / lengthPerRecord;
    int blockIndex = (recordOffset) / recordPerBlock + 1;
    int recordOffsetInBlock = (recordOffset) % recordPerBlock;
    bf.Unlock(blockNum);

    int recordNum = bf.FindBlock(name, blockIndex);
    char *recordData = bf.ReadBlockData(recordNum);
    bool valid = *(bool *)(recordData + recordOffsetInBlock * lengthPerRecord + ValidPos);
    bf.Unlock(recordNum);
    return valid;
}

void RecordManager::Create(Table table)
{
    std::string name = GetFileName(table);
    int blockNum = bf.FindBlock(name, 0);
    int maxOffset = 0;
    int lengthPerRecord = AttPos + 4 * table.attrs.size();
    for (int i = 0; i < table.attr_num; i++)
    {
        lengthPerRecord += (table.attrs[i].attr_type > 0 ? table.attrs[i].attr_type : 4);
    }
    bf.WriteData(blockNum, (char *)&maxOffset, MaxOffsetPos, 4);
    bf.WriteData(blockNum, (char *)&lengthPerRecord, LengthPos, 4);
    bf.Unlock(blockNum);
    int recordNum = bf.FindBlock(name, 1);
    bf.Unlock(recordNum);
}

int RecordManager::Insert(Table table, Record record)
{
    std::string name = GetFileName(table);
    int blockNum = bf.FindBlock(name, 0);
    char *data = bf.ReadBlockData(blockNum);
    int maxOffset = *(int *)(data + MaxOffsetPos);
    int lengthPerRecord = *(int *)(data + LengthPos);
    int recordPerBlock = BlockMaxSize / lengthPerRecord;
    int blockIndex = (maxOffset) / recordPerBlock + 1;
    int recordOffset = (maxOffset) % recordPerBlock;

    int recordNum = bf.FindBlock(name, blockIndex);
    char newData[BlockMaxSize];
    bool valid = true;
    char nullMap[4] = {0};
    memcpy(newData + ValidPos, (char *)&valid, 1);
    memcpy(newData + NullPos, nullMap, 8);
    short recordPos = AttPos + 4 * table.attr_num;
    for (int i = 0; i < table.attrs.size(); i++)
    {
        short type = table.attrs[i].attr_type;
        short length = type > 0 ? type : 4;
        memcpy(newData + AttPos + i * 4, (char *)&recordPos, 2);
        memcpy(newData + AttPos + i * 4 + 2, (char *)&length, 2);
        const char *key = record.atts[i];
        memcpy(newData + recordPos, key, length);
        if (record.null[i])
        {
            *(newData + NullPos + i / 8) |= bitMap[i % 8];
        }
        recordPos+=length;
    }
    bf.WriteData(recordNum, newData, recordOffset * lengthPerRecord, lengthPerRecord);
    bf.Unlock(recordNum);
    maxOffset++;
    bf.WriteData(blockNum, (char *)&maxOffset, MaxOffsetPos, 4);
    bf.Unlock(blockNum);
    return maxOffset;
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
    bf.Unlock(blockNum);
    int recordNum = bf.FindBlock(name, blockIndex);
    bool valid = false;
    bf.WriteData(recordNum, (char *)&valid, lengthPerRecord * recordOffsetInBlock + ValidPos, 1);
    bf.Unlock(recordNum);
}

void RecordManager::ReadRecord(Table table, int recordOffset, Record& record)
{
    std::string name = GetFileName(table);
    int blockNum = bf.FindBlock(name, 0);
    char *data = bf.ReadBlockData(blockNum);
    int lengthPerRecord = *(int *)(data + LengthPos);
    int recordPerBlock = BlockMaxSize / lengthPerRecord;
    int blockIndex = (recordOffset) / recordPerBlock + 1;
    int recordOffsetInBlock = (recordOffset) % recordPerBlock;
    bf.Unlock(blockNum);
    int recordNum = bf.FindBlock(name, blockIndex);
    char *recordData = bf.ReadBlockData(recordNum);
    for (int i = 0; i < table.attr_num; i++)
    {
        short pos = *(short *)(recordData + recordOffsetInBlock * lengthPerRecord + AttPos + 4 * i);
        short length = *(short *)(recordData + recordOffsetInBlock * lengthPerRecord + AttPos + 4 * i + 2);
        memcpy(record.atts[i], recordData + recordOffsetInBlock * lengthPerRecord + pos,length);
        char *nullMap = recordData + recordOffsetInBlock * lengthPerRecord + NullPos;
        bool isNull = *(nullMap + i / 8) & bitMap[i % 8];
        record.null.push_back(isNull);
    }
    bf.Unlock(recordNum);
}

void RecordManager::DropRecordFile(Table table)
{
    std::string name=GetFileName(table);
    bf.DropFile(name);
}
