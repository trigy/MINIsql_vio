#include "API.h"

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
        // cout<<"primary key: "<<id<<endl;
        //create index automatically
        if (id != -1)
        {
            string index_name = table.attrs[id].attr_name;
            Index idx(index_name,table.table_name,table.attrs[id].attr_name);

            //catalog create index
            CTM.CreateIndex(idx);
            //index manager create index
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
    CTM.DropTable(tb);
    RCM.DropRecordFile(tb);
    vector<Index> indexList;
    CTM.GetIndexList(tb,indexList);
    for(int i=0;i<indexList.size();i++)
    {
        IDM.DropIndexFile(indexList[i]); 
    }
    cout << "QUERY OK, 0 rows affected" << endl;
}

//when create Index
void API_Create_Index(Index &index)
{
    int x = CTM.IndexOffset(index.table_name, index.index_name);
    if (x == -1)
    {
        CTM.CreateIndex(index);
        int i;
        //select firstly
        //select* from table_name

        Table tb = CTM.ReadTable(index.table_name);
        for (i = 0; i < tb.attr_num; i++)
            if (tb.attrs[i].attr_name == index.attr_name)
                break;
        //      CreateIndex(values, index.index_name, tb.attrs[ID].attr_type - INT);
        IDM.CreateIndexHead(index, tb.attrs[i].attr_type);
        cout << "QUERY OK, 0 rows affected" << endl;
    }
    else
    {
        cout << "ERROR : index has already exists" << endl;
    }
}

//when delete Index
// void API_DropIndex(Index index)
// {
//     //catalog drop index
//     int offset = CTM.IndexOffset(index.table_name, index.index_name);
//     if (offset != -1)
//     {
//         CTM.DropIndex(index.table_name, offset);
//         cout << "QUERY OK, 0 rows affected" << endl;
//     }
//     else
//         cout << "ERROR: Index " << index.index_name << " doesn't exist" << endl;
// }

void API_DropIndex(string table_name, string index_name)
{
    //catalog drop index
    int offset = CTM.IndexOffset(table_name, index_name);
    if (offset != -1)
    {
        CTM.DropIndex(table_name, offset);
        cout << "QUERY OK, 0 rows affected" << endl;
    }
    else
        cout << "ERROR: Index " << index_name << " doesn't exist" << endl;
}

void API_Insert(Table &table ,Record& record)
{
    int rf=RCM.Insert(table,record);
    vector<Index> indexList;
    if(CTM.GetIndexList(table,indexList))
    {
        for(int i=0;i<indexList.size();i++)
        {
            for(int j=0;j<table.attrs.size();j++)
            {
                if(table.attrs[j].attr_name==indexList[i].attr_name)
                {
                    IDM.Insert(indexList[i],record.atts[j],rf);
                }
            }
        }
    }
    //插入index
    // cout << "QUERY OK, 1 rows affected" << endl;
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
                //不考虑重复的属性
                find=true;
            }
        }
        if(!find)
        {
            cerr<<"ERROR: can not find Attritube "<<selectAttr[i]<<endl;
            return;
        }
    }
    cout<<"#\t";
    for(int i=0;i<selectAttr.size();i++)
    {
        cout<<selectAttr[i]<<"\t";
    }
    cout<<endl;
    int maxRecord=RCM.MaxOffset(table);
    std::cout<<"table has "<<maxRecord<<" records"<<std::endl;
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
                        // char term[256];
                        // memcpy(term, record.atts[index], table.attrs[index].attr_type);
                        // term[table.attrs[index].attr_type]='\0';
                        cout << string(record.atts[index]) << "\t";
                    }
                }
            }
            cout << endl;
        }   
    }
    cout << "QUERY OK, 0 rows affected" << endl;
}

int cmp(char *da, char* db, short type)
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
                return 0;
        }
    }
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
                //不考虑重复的属性
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
                // cout<<id<<endl;
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
                    for(int k=0;k<=maxRecord;k++)
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
                    // char term[256];
                    // memcpy(term, record.atts[index], table.attrs[index].attr_type);
                    // term[table.attrs[index].attr_type]='\0';
                    cout << string(record.atts[index]) << "\t";
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

}

//     //inside function when operate on the log
// void API_Select(string table_name, vector<string> &attr, string WClasue)
// {
//     if (!CTM.TableExist(table_name))
//     {
//         cerr << "ERROR:"
//              << "NO SUCH TABLE!" << endl;
//         return;
//     }
//     Table tb = CTM.ReadTable(table_name);
//     Record record;
//     int i = 0, count;
//     int pos1 = 0, pos2;
//     int end = 0;
//     ConditionList Slist;
//     while ((i = WClasue.find(' ', i)) != string::npos)
//         WClasue.erase(i, 1);
//     //delete the space
//     while (1)
//     {
//         size_t a = min(WClasue.find("==", pos1), WClasue.find(">=", pos1));
//         a = min(a, WClasue.find(">=", pos1));
//         a = min(a, WClasue.find(">", pos1));
//         a = min(a, WClasue.find("<", pos1));
//         a = min(a, WClasue.find("<=", pos1));
//         a = min(a, WClasue.find("<>", pos1));
//         if ((int)a == -1)
//         {
//             if (Slist.empty())
//             {
//                 cout << "ERROR: invalid condition after 'where' " << endl;
//                 return;
//             }
//         }
//         string attr_name, op, com_val;
//         switch (WClasue[a])
//         {
//         case '>':
//             if (WClasue[a + 1] == '=')
//                 op = WClasue.substr(a, 2);
//             else
//                 op = WClasue.substr(a, 1);
//             break;
//         case '<':
//             if (WClasue[a + 1] == '=' || WClasue[a + 1] == '>')
//                 op = WClasue.substr(a, 2);
//             else
//                 op = WClasue.substr(a, 1);
//             break;
//         case '=':
//             op = WClasue.substr(a, 2);
//             break;
//         default:
//             break;
//         }
//         attr_name = WClasue.substr(pos1, a - pos1);
//         pos2 = WClasue.find("and", pos1);
//         if (pos2 == -1)
//             break;
//         pos1 = a + op.length();
//         if (WClasue[pos1] == '\'')
//             com_val = WClasue.substr(pos1 + 1, pos2 - 2 - pos1);
//         else
//             com_val = WClasue.substr(pos1 + 1, pos2 - pos1);
//         pos1 = pos2 + 3;
//         Condition NCon(attr_name, op, com_val);
//         Slist.push_back(NCon);
//     }

//     i = 0;
//     vector<int> vd = Select(Slist[i].attr_name, Slist[i].cmp_value, Slist[i].cmp_value);
//     sort(vd.begin(), vd.end());
//     vd.erase(unique(vd.begin(), vd.end()), vd.end());
//     vector<int>::iterator mIter;
//     for (i = 1; i < Slist.size(); i++)
//     {
//         vector<int> vb = Select(Slist[i].attr_name, Slist[i].cmp_value, Slist[i].cmp_value);
//         vector<int> va = vd;
//         //vd = va jiao vb, va = vd
//         sort(va.begin(), va.end());
//         sort(vb.begin(), vb.end());
//         vd.resize(va.size() + vb.size());
//         mIter = set_intersection(va.begin(), va.end(), vb.begin(), vb.end(), vd.begin());
//         vd.resize(mIter - vd.begin());
//         //delete space
//         sort(vd.begin(), vd.end());
//         vd.erase(unique(vd.begin(), vd.end()), vd.end());
//     }
//     if (vd.empty())
//     {
//         cout << "Empty Set" << endl;
//         return;
//     }
//     count = vd.size();
//     API_DrawResult();
//     cout << count << " rows in set" << endl;
// }

// void API_Delete(string table_name, string WClasue) //undo
// {
//     if (!CTM.TableExist(table_name))
//     {
//         cerr << "ERROR:"
//              << "NO SUCH TABLE!" << endl;
//         return;
//     }
//     Table tb = CTM.ReadTable(table_name);
//     Record record;
//     int i = 0, count;
//     int pos1 = 0, pos2;
//     int end = 0;
//     ConditionList Slist;
//     while ((i = WClasue.find(' ', i)) != string::npos)
//         WClasue.erase(i, 1);
//     //delete the space
//     while (1)
//     {
//         size_t a = min(WClasue.find("==", pos1), WClasue.find(">=", pos1));
//         a = min(a, WClasue.find(">=", pos1));
//         a = min(a, WClasue.find(">", pos1));
//         a = min(a, WClasue.find("<", pos1));
//         a = min(a, WClasue.find("<=", pos1));
//         a = min(a, WClasue.find("<>", pos1));
//         if ((int)a == -1)
//         {
//             if (Slist.empty())
//             {
//                 cout << "ERROR: invalid condition after 'where' " << endl;
//                 return;
//             }
//         }
//         string attr_name, op, com_val;
//         switch (WClasue[a])
//         {
//         case '>':
//             if (WClasue[a + 1] == '=')
//                 op = WClasue.substr(a, 2);
//             else
//                 op = WClasue.substr(a, 1);
//             break;
//         case '<':
//             if (WClasue[a + 1] == '=' || WClasue[a + 1] == '>')
//                 op = WClasue.substr(a, 2);
//             else
//                 op = WClasue.substr(a, 1);
//             break;
//         case '=':
//             op = WClasue.substr(a, 2);
//             break;
//         default:
//             break;
//         }
//         attr_name = WClasue.substr(pos1, a - pos1);
//         pos2 = WClasue.find("and", pos1);
//         if (pos2 == -1)
//             break;
//         pos1 = a + op.length();
//         if (WClasue[pos1] == '\'')
//             com_val = WClasue.substr(pos1 + 1, pos2 - 2 - pos1);
//         else
//             com_val = WClasue.substr(pos1 + 1, pos2 - pos1);
//         pos1 = pos2 + 3;
//         Condition NCon(attr_name, op, com_val);
//         Slist.push_back(NCon);
//     }
//     i = 0;
//     vector<int> vd = Select(Slist[i].attr_name, Slist[i].cmp_value, Slist[i].cmp_value);
//     sort(vd.begin(), vd.end());
//     vd.erase(unique(vd.begin(), vd.end()), vd.end());
//     vector<int>::iterator mIter;
//     for (i = 1; i < Slist.size(); i++)
//     {
//         vector<int> vb = Select(Slist[i].attr_name, Slist[i].cmp_value, Slist[i].cmp_value);
//         vector<int> va = vd;
//         //vd = va jiao vb, va = vd
//         sort(va.begin(), va.end());
//         sort(vb.begin(), vb.end());
//         vd.resize(va.size() + vb.size());
//         mIter = set_intersection(va.begin(), va.end(), vb.begin(), vb.end(), vd.begin());
//         vd.resize(mIter - vd.begin());
//         //delete space
//         sort(vd.begin(), vd.end());
//         vd.erase(unique(vd.begin(), vd.end()), vd.end());
//     }
//     if (vd.empty())
//     {
//         cout << "Empty Set" << endl;
//         return;
//     }

//     for (int i = 0; i < vd.size(); i++)
//         RCM.Delete(tb, vd[i]);
//     cout << "Query OK, " << i << " rows affected" << endl;
// }

// vector<string> split(string str, string pattern)
// {
//     string::size_type pos;
//     vector<string> res;
//     str += pattern;
//     int i;
//     int size = str.size();
//     for (i = 0; i < size; i++)
//     {
//         pos = str.find(pattern, i);
//         if (pos < size)
//         {
//             string s = str.substr(i, pos - i);
//             res.push_back(s);
//             i = pos + pattern.size() - 1;
//         }
//     }
//     return res;
// }

// string split(string str, string pattern, int id)
// {
//     vector<string> res = split(str, pattern);
//     return res[id];
// }

// void API_DrawResult(Table &tb, Record res)
// {
//     static int Width[32];
//     int i;
//     vector<string>::iterator it = select_values.begin();
//     while (it != select_values.end())
//     {
//         vector<string> vs = split(*it, "\t");
//         for (i = 0; i < tb.attr_num; i++)
//         {
//             if (vs[i].length() > Width[i])
//                 Width[i] = vs[i].length();
//         }
//         it++;
//     }

//     for (i = 0; i < tb.attr_num; i++)
//     {
//         if (tb.attrs[i].attr_name.length() > Width[i])
//             Width[i] = tb.attrs[i].attr_name.length();
//     }
//     for (i = 0; i < tb.attr_num; i++)
//         Width[i] += 2;

//     //draw
// }

//merry the time
double Cal_time(long start, long end)
{
    return (double)(end - start) / CLK_TCK;
}
