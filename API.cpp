#include "API.h"

int cmp(char *da, char *db, short type)
{
    if (type == 0) //int
    {
        int a = *(int *)da;
        int b = *(int *)db;
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
    else if (type == -1) //float
    {
        float a = *(float *)da;
        float b = *(float *)db;
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
    else
    {
        for (short i = 0; i < type; i++)
        {
            if (*(da + i) > *(db + i))
                return 1;
            else if (*(da + i) < *(db + i))
                return -1;
            else
                continue;
        }
        return 0;
    }
}
//when create table
void API_CreateTable(Table &table)
{
    if (CTM.TableExist(table.table_name))
    {
        cout << "ERROR: Table " << table.table_name << " has already exist." << endl;
    }
    else
    {
        CTM.CreateTable(table);
        RCM.Create(table);
        int id = table.getPrimaryKeyId();
        if (id != -1)
        {
            string index_name = table.attrs[id].attr_name;
            Index idx(index_name,table.table_name,table.attrs[id].attr_name);

            CTM.CreateIndex(idx);
            IDM.CreateIndexHead(idx, table.attrs[id].attr_type);
        }

        cout << "QUERY OK, 0 rows affected" << endl;
    }
}

//when delete the Table
void API_DropTable(string table_name)
{

    if (CTM.TableExist(table_name) == 0)
    {
        cout << "ERROR: Table " << table_name << " doesn't exist" << endl;
        return;
    }
    Table tb = CTM.ReadTable(table_name);
    RCM.DropRecordFile(tb);
    vector<Index> indexList;
    CTM.GetIndexList(tb,indexList);
    for(int i=0;i<indexList.size();i++)
    {
        IDM.DropIndexFile(indexList[i]); 
    }
    CTM.DropTable(tb);
    cout << "QUERY OK, 0 rows affected" << endl;
}

//when create Index
void API_CreateIndex(Index &index)
{
    int x = CTM.IndexOffset(index.table_name, index.index_name);
    if (x == -1)
    {
        CTM.CreateIndex(index);
        int i;
        Table tb = CTM.ReadTable(index.table_name);
        for (i = 0; i < tb.attr_num; i++)
            if (tb.attrs[i].attr_name == index.attr_name && tb.attrs[i].attr_key_type!=OTHER)
                break;
        IDM.CreateIndexHead(index, tb.attrs[i].attr_type);
        cout << "QUERY OK, 0 rows affected" << endl;
    }
    else
    {
        cout << "ERROR : index has already exists" << endl;
    }
}

void API_DropIndex(string table_name, string index_name)
{
    int offset = CTM.IndexOffset(table_name, index_name);
    if (offset != -1)
    {
        CTM.DropIndex(table_name, offset);
        Index index;
        index.table_name=table_name;
        index.index_name=index_name;
        IDM.DropIndexFile(index);
        cout << "QUERY OK, 0 rows affected" << endl;
    }
    else
        cout << "ERROR: Index " << index_name << " doesn't exist" << endl;
}

void API_Insert(Table &table ,Record& record)
{
    int rf=RCM.Insert(table,record);
    vector<Index> indexList;
    CTM.GetIndexList(table,indexList);
    for(int i=0;i<table.attrs.size();i++)
    {
        if(table.attrs[i].attr_key_type!=OTHER)
        {
            bool find=false;
            for( int j=0;j<indexList.size();j++)
            {
                if (table.attrs[i].attr_name == indexList[j].attr_name)
                {
                    if (IDM.Insert(indexList[j], record.atts[i], rf) == -1)
                    {
                        RCM.Delete(table, rf - 1);
                        cerr << "ERROR: conflict with unique attribute" << endl;
                        return;
                    }
                    find=true;
                }
            }
            if(find)
            {
                continue;
            }
            for (int j = 0; j <RCM.MaxOffset(table)-1; j++)
            {
                Record orecord;
                RCM.ReadRecord(table,j,orecord);
                if (!cmp(record.atts[i],orecord.atts[i],table.attrs[i].attr_type))
                {
                    RCM.Delete(table, rf - 1);
                    cerr << "ERROR: conflict with unique attribute" << endl;
                    return;
                }
            }
        }
    }
}

void API_SelectAll(Table &table, vector<string> &selectAttr)
{
    
    vector<int> attrNo;
    for(int i=0;i<selectAttr.size();i++)
    {
        bool find=false;
        for(int j=0;j<table.attrs.size();j++)
        {
            if (table.attrs[j].attr_name == selectAttr[i])
            {
                attrNo.push_back(j);
                find=true;
            }
        }
        if(!find)
        {
            cerr<<"ERROR: can not find Attritube "<<selectAttr[i]<<endl;
            return;
        }
    }
    int maxRecord=RCM.MaxOffset(table);
    cout<<"#\t";
    for(int i=0;i<selectAttr.size();i++)
    {
        cout<<selectAttr[i]<<"\t";
    }
    cout<<endl;
    int num=0;
    for(int i=0;i<maxRecord;i++)
    {
        if(RCM.IsValid(table,i))
        {
            cout<<num<<"\t";
            num++;
            Record record;
            RCM.ReadRecord(table,i,record);
            for(int j=0;j<attrNo.size();j++)
            {
                int index=attrNo[j];
                if(record.null[index]==true)
                {
                    cout<<"\t";
                }
                else 
                {
                    if (table.attrs[index].attr_type == INT)
                    {
                        cout<<*(int *)(record.atts[index])<<"\t";
                    }
                    else if (table.attrs[index].attr_type == FLOAT)
                    {
                        cout << *(float *)(record.atts[index]) << "\t";
                    }
                    else 
                    {
                        for (int j = 0; j < table.attrs[index].attr_type; j++)
                        {
                            cout << record.atts[index][j];
                        }
                        cout<< "\t";
                    }
                }
            }
            cout << endl;
        }   
    }
    cout << "QUERY OK, 0 rows affected" << endl;
}

void API_SelectCon(Table &table, vector<string> &selectAttr, ConditionList &cl)
{

    vector<int> attrNo;
    for (int i = 0; i < selectAttr.size(); i++)
    {
        bool find = false;
        for (int j = 0; j < table.attrs.size(); j++)
        {
            if (table.attrs[j].attr_name == selectAttr[i])
            {
                attrNo.push_back(j);
                find = true;
            }
        }
        if (!find)
        {
            cerr << "ERROR: can not find Attritube " << selectAttr[i] << endl;
            return;
        }
    }
    vector<int> selectOffset;
    vector<int> findCount;
    int maxRecord = RCM.MaxOffset(table);
    for(int i=0;i<maxRecord;i++)
    {
        findCount.push_back(0);
    }
    for(int i=0;i<cl.size();i++)
    {
        bool find = false;
        for (int j = 0; j < table.attrs.size(); j++)
        {
            if (table.attrs[j].attr_name == cl[i].attr_name)
            {
                int id=CTM.IndexOffset2(table.table_name,table.attrs[j].attr_name);
                if(id>=0) //如果在这个属性上建立了索引
                {
                    Index index = CTM.ReadIndex(table.table_name, table.attrs[j].attr_name,id);
                        if(cl[i].operation==EQU)
                        {
                            bool exist;
                            int rf = IDM.Search(index, cl[i].cmp_value, exist);
                            if (exist)
                            {
                                findCount[rf-1]++;
                                if (findCount[rf-1] == cl.size())
                                {
                                    selectOffset.push_back(rf-1);
                                }        
                            }
                    
                        }
                        else if (cl[i].operation==NEQ)
                        {
                            bool exist;
                            int rf = IDM.Search(index, cl[i].cmp_value, exist);
                            for(int i=0;i<findCount.size();i++)
                            {
                                if(i!=rf-1||!exist)
                                {
                                    findCount[i]++;
                                    if (findCount[i] == cl.size())
                                    {
                                        selectOffset.push_back(i);
                                    }
                                }
                            }
                        }
                        else if (cl[i].operation==LGE)
                        {
                            vector<int>valList;
                            IDM.SearchLarger(index,cl[i].cmp_value,valList,true);
                            for(int m=0;m<valList.size();m++)
                            {
                                findCount[valList[m]-1]++;
                                if (findCount[valList[m]-1] == cl.size())
                                {
                                    selectOffset.push_back(valList[m]-1);
                                }
                            }
                        }
                        else if(cl[i].operation==SME)                         
                        {
                            vector<int> valList;
                            IDM.SearchSmaller(index, cl[i].cmp_value, valList, true);
                            for (int m = 0; m < valList.size(); m++)
                            {
                                findCount[valList[m] - 1]++;
                                if (findCount[valList[m] - 1] == cl.size())
                                {
                                    selectOffset.push_back(valList[m] - 1);
                                }
                            }
                        }
                        else if (cl[i].operation == LRG)
                        {
                            vector<int> valList;
                            IDM.SearchLarger(index, cl[i].cmp_value, valList, false);
                            for (int m = 0; m < valList.size(); m++)
                            {
                                findCount[valList[m] - 1]++;
                                if (findCount[valList[m] - 1] == cl.size())
                                {
                                    selectOffset.push_back(valList[m] - 1);
                                }
                            }
                        }
                        else if (cl[i].operation == SML)
                        {
                            vector<int> valList;
                            IDM.SearchSmaller(index, cl[i].cmp_value, valList, false);
                            for (int m = 0; m < valList.size(); m++)
                            {
                                findCount[valList[m] - 1]++;
                                if (findCount[valList[m] - 1] == cl.size())
                                {
                                    selectOffset.push_back(valList[m] - 1);
                                }
                            }
                        }
                }
                else //如果没有建立索引
                {
                    for(int k=0;k<maxRecord;k++)
                    {
                        if(RCM.IsValid(table,k))
                        {
                            Record record;
                            RCM.ReadRecord(table,k,record);
                            int res = cmp(record.atts[j], cl[i].cmp_value, table.attrs[j].attr_type);
                            if ((cl[i].operation == EQU && res == 0) || (cl[i].operation == NEQ && res != 0) || (cl[i].operation == LGE && res >= 0) 
                            || (cl[i].operation == SME && res <= 0) || (cl[i].operation == LRG && res > 0) || (cl[i].operation == SML && res < 0))
                            {
                                findCount[k]++;
                                if(findCount[k]==cl.size())
                                {
                                    selectOffset.push_back(k);
                                }
                            }
                        }
                    }
                }
                find = true;
            }
        }
        if (!find)
        {
            cerr << "ERROR: can not find Attritube " << selectAttr[i] << endl;
            return;
        }
    }
    
    cout << "#\t";
    for (int i = 0; i < selectAttr.size(); i++)
    {
        cout << selectAttr[i] << "\t";
    }
    cout << endl;
    int num = 0;
    for (int i = 0; i < selectOffset.size(); i++)
    {
        cout << num << "\t";
        num++;
        Record record;
        RCM.ReadRecord(table, selectOffset[i], record);
        for (int j = 0; j < attrNo.size(); j++)
        {
            int index = attrNo[j];
            if (record.null[index] == true)
            {
                cout << "NULL\t";
            }
            else
            {
                if (table.attrs[index].attr_type == INT)
                {
                    cout << *(int *)(record.atts[index]) << "\t";
                }
                else if (table.attrs[index].attr_type == FLOAT)
                {
                    cout << *(float *)(record.atts[index]) << "\t";
                }
                else
                {
                    for (int j = 0; j < table.attrs[index].attr_type; j++)
                    {
                        cout << record.atts[index][j];
                    }
                    cout << "\t";
                }
            }
        }
        cout << endl;
    }
    cout << "QUERY OK, 0 rows affected" << endl;
}

void API_DeleteAll(Table &table)
{
    int maxRecord = RCM.MaxOffset(table);
    for (int i = 0; i <= maxRecord; i++)
    {
        RCM.Delete(table,i);
    }
    vector<Index> indexList;
    CTM.GetIndexList(table,indexList);
    for(int i=0;i<indexList.size();i++)
    {
        IDM.DropIndexFile(indexList[i]);
    }
    cout << "QUERY OK, 0 rows affected" << endl;
}

void API_DeleteCon(Table &table, ConditionList &cl)
{
    vector<int> selectOffset;
    vector<int> findCount;
    int maxRecord = RCM.MaxOffset(table);
    for (int i = 0; i < maxRecord; i++)
    {
        findCount.push_back(0);
    }
    for (int i = 0; i < cl.size(); i++)
    {
        bool find = false;
        for (int j = 0; j < table.attrs.size(); j++)
        {
            if (table.attrs[j].attr_name == cl[i].attr_name)
            {
                int id = CTM.IndexOffset2(table.table_name, table.attrs[j].attr_name);
                if (id >= 0) //如果在这个属性上建立了索引
                {
                    Index index = CTM.ReadIndex(table.table_name, table.attrs[j].attr_name, id);
                    if (cl[i].operation == EQU)
                    {
                        bool exist;
                        int rf = IDM.Search(index, cl[i].cmp_value, exist);
                        if (exist)
                        {
                            findCount[rf - 1]++;
                            if (findCount[rf - 1] == cl.size())
                            {
                                selectOffset.push_back(rf - 1);
                            }
                        }
                    }
                    else if (cl[i].operation == NEQ)
                    {
                        bool exist;
                        int rf = IDM.Search(index, cl[i].cmp_value, exist);
                        for (int i = 0; i < findCount.size(); i++)
                        {
                            if (i != rf - 1 || !exist)
                            {
                                findCount[i]++;
                                if (findCount[i] == cl.size())
                                {
                                    selectOffset.push_back(i);
                                }
                            }
                        }
                    }
                    else if (cl[i].operation == LGE)
                    {
                        vector<int> valList;
                        IDM.SearchLarger(index, cl[i].cmp_value, valList, true);
                        for (int m = 0; m < valList.size(); m++)
                        {
                            findCount[valList[m] - 1]++;
                            if (findCount[valList[m] - 1] == cl.size())
                            {
                                selectOffset.push_back(valList[m] - 1);
                            }
                        }
                    }
                    else if (cl[i].operation == SME)
                    {
                        vector<int> valList;
                        IDM.SearchSmaller(index, cl[i].cmp_value, valList, true);
                        for (int m = 0; m < valList.size(); m++)
                        {
                            findCount[valList[m] - 1]++;
                            if (findCount[valList[m] - 1] == cl.size())
                            {
                                selectOffset.push_back(valList[m] - 1);
                            }
                        }
                    }
                    else if (cl[i].operation == LRG)
                    {
                        vector<int> valList;
                        IDM.SearchLarger(index, cl[i].cmp_value, valList, false);
                        for (int m = 0; m < valList.size(); m++)
                        {
                            findCount[valList[m] - 1]++;
                            if (findCount[valList[m] - 1] == cl.size())
                            {
                                selectOffset.push_back(valList[m] - 1);
                            }
                        }
                    }
                    else if (cl[i].operation == SML)
                    {
                        vector<int> valList;
                        IDM.SearchSmaller(index, cl[i].cmp_value, valList, false);
                        for (int m = 0; m < valList.size(); m++)
                        {
                            findCount[valList[m] - 1]++;
                            if (findCount[valList[m] - 1] == cl.size())
                            {
                                selectOffset.push_back(valList[m] - 1);
                            }
                        }
                    }
                }
                else //如果没有建立索引
                {
                    for (int k = 0; k < maxRecord; k++)
                    {
                        if (RCM.IsValid(table, k))
                        {
                            Record record;
                            RCM.ReadRecord(table, k, record);
                            int res = cmp(record.atts[j], cl[i].cmp_value, table.attrs[j].attr_type);
                            if ((cl[i].operation == EQU && res == 0) || (cl[i].operation == NEQ && res != 0) || (cl[i].operation == LGE && res >= 0) || (cl[i].operation == SME && res <= 0) || (cl[i].operation == LRG && res > 0) || (cl[i].operation == SML && res < 0))
                            {
                                findCount[k]++;
                                if (findCount[k] == cl.size())
                                {
                                    selectOffset.push_back(k);
                                }
                            }
                        }
                    }
                }
                find = true;
            }
        }
    }
    for (int i = 0; i < selectOffset.size(); i++)
    {
        Record record;
        RCM.ReadRecord(table,selectOffset[i],record);
        RCM.Delete(table,selectOffset[i]);
        vector<Index> indexList;
        CTM.GetIndexList(table, indexList);
        for (int j = 0; j < indexList.size(); j++)
        {
            for (int k = 0; k < table.attrs.size(); k++)
            {
                if (table.attrs[k].attr_name == indexList[j].attr_name)
                {
                    IDM.Delete(indexList[j], record.atts[k]);
                }
            }  
        }
    }
    cout << "QUERY OK, "<<selectOffset.size()<<" rows affected" << endl;
}
