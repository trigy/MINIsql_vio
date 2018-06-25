#include "API.h"

using namespace std;

extern RecordManager RCM;
extern CatalogManager CTM;
extern IndexManager IDM;
//when create table 
void API_CreateTable(Table& table)
{
    if(CTM.TableExist(table.table_name))
    {
        cout<<"ERROR: Table "<<table.table_name<<" has already exist."<<endl;
    }
	else
	{
        CTM.CreateTable(table);
		int id = table.getPrimaryKeyId();
        //create index automatically
        if(id != -1)
        {
            string index_name = table.table_name + "_" +table.attrs[id].attr_name;
            Index idx(table.table_name,table.attrs[id].attr_name,index_name);
            /*while(idx.index_name == Find_index_name(table.table_name,table.attrs[id].attr_name))
            {
                idx.index_name = "_" + idx.index_name;
            }*/
            
            //catalog create index
            CTM.CreateIndex(idx);
            //index manager create index
            IDM.CreateIndexHead(idx,table.attrs[id].attr_type);
        }
        
        cout<<"QUERY OK, 0 rows affected"<<endl;
		
	}
	
}


//when delete the Table
void API_DropTable(string table_name)
{
    
    if(CTM.TableExist(table_name) == 0)
    {
        cout<<"ERROR: Table " << table_name <<" doesn't exist"<<endl;
        return;
    }
    Table tb = CTM.ReadTable(table_name);
    CTM.DropTable(tb);
    cout<<"QUERY OK, 0 rows affected"<<endl;
        
}


//when create Index
void API_Create_Index(Index& index)
{
    int x = CTM.IndexOffset(index.table_name, index.index_name);
    CTM.CreateIndex(index);
    if(x == -1)
    {
        int i;
        //select firstly
        //select* from table_name
        
        Table tb = CTM.ReadTable(index.table_name);
        for(i = 0 ; i < tb.attr_num; i++)
            if(tb.attrs[i].attr_name == index.attr_name)
                break;
//      CreateIndex(values, index.index_name, tb.attrs[ID].attr_type - INT);
        IDM.CreateIndexHead(index, tb.attrs[i].attr_type);
        cout<<"QUERY OK, 0 rows affected"<<endl;
    }
    else
}


//when delete Index
void API_DropIndex(Index index)
{
    //catalog drop index
    int offset = CTM.IndexOffset(index.table_name,index.index_name);
    if(offset != -1)
    {
        CTM.DropIndex(index.table_name,offset);
        cout<<"QUERY OK, 0 rows affected"<<endl;
        
    }
    else
        cout<<"ERROR: Index " << index.index_name <<" doesn't exist"<<endl;
}



//tb
//insert
void API_Insert(string table_name, string Values) //error
{
    if(!CTM.TableExist(table_name))
    {
        cerr<<"ERROR:"<<"NO SUCH TABLE!"<<endl;
        return;
    }
    Table tb = CTM.ReadTable(table_name);
    Record record;
    int i,j;
    int pos1 = 0,pos2;
    int end = 0;
    Values.erase(remove_if(Values.begin(), Values.end(), [](char x) { return isspace(x); }), Values.end());
    for( i = 0; i < tb.attr_num - 1; i++)
    {
        if(tb.attrs[i].attr_type == INT || tb.attrs[i].attr_type == FLOAT)
        {
            if(Values[pos1] == '\'' )
            {
                cerr<<"ERROR:"<<i<<"'s attribute require number"<<endl;
                return ;
            }
            pos2 = Values.find(',',pos1);
            if(pos2 == -1)
            {
                cerr<<"ERROR: not enough values"<<endl;
                return ;
            }
            end = pos2;
            string x = Values.substr(pos1,pos2-1-pos1);
            if(atof(x.c_str()) == 0)
            {
                cerr<<"ERROR:"<<i<<"'s attribute require number"<<endl;
            }
            record.atts.push_back(x);
            pos1 = end +1;
        }
        else
        {
            if(Values[pos1] != '\'' )
            {
                cerr<<"ERROR:"<<i<<"'s attribute require char"<<endl;
                return ;
            }
            pos2 = Values.find('\'',pos1+1);
            if(Values[pos2+1] != ',')
            {
                cerr<<"ERROR: Synaxt error near '''"<<endl;
                return ;
            }
            end = pos2+1;
            string x = Values.substr(pos1,pos2-1-pos1);
            if(x.length() != tb.attrs[i].attr_type)
            {
                cerr<<"ERROR: wrong bits char"<<endl;
                return ;
            }
            record.atts.push_back(x);
            pos1 = end + 1;
        }
        //set null
        if(tb.attrs[i].attr_key_type != OTHER)
            record.null.push_back(false);
        else record.null.push_back(true);
    }
    if(tb.attrs[i].attr_type == INT || tb.attrs[i].attr_type == FLOAT)
    {
        if(Values[pos1] == '\'' )
        {
            cerr<<"ERROR:"<<i<<"'s attribute require number"<<endl;
            return ;
        }
        pos2 = Values.length();
        if(pos2 - pos1 <= 0)
        {
            cerr<<"ERROR: not enough values"<<endl;
            return ;
        }
        string x = Values.substr(pos1,pos2-pos1);
        if(atof(x.c_str()) == 0)
        {
            cerr<<"ERROR:"<<i<<"'s attribute require number"<<endl;
        }
        record.atts.push_back(x);
    }
    else
    {
        if(Values[pos1] != '\'' )
        {
            cerr<<"ERROR:"<<i<<"'s attribute require char"<<endl;
            return ;
        }
        pos2 = Values.find('\'',pos1+1);
        if(pos2 - pos1 <= 0)
        {
            cerr<<"ERROR: not enough values"<<endl;
            return ;
        }
        string x = Values.substr(pos1+1,pos2-2-pos1);
        if(x.length() != tb.attrs[i].attr_type)
        {
            cerr<<"ERROR: wrong bits char"<<endl;
            return ;
        }
        record.atts.push_back(x);
    }
    //set null
    if(tb.attrs[i].attr_key_type != OTHER)
        record.null.push_back(false);
    else record.null.push_back(true);
    for(i = 0; i < tb.attr_num; i++)
    {
        //check all attr
        if(tb.attrs[i].attr_key_type == PRIMARY)
        {
            //get index_name
            int offset = CTM.IndexOffset(tb.table_name, tb.attrs[i].attr_name);
            Index idx = CTM.ReadIndex(tb.table_name, tb.attrs[i].attr_name, offset);
            //search if same name
            vector<int> v0 = Select(idx.index_name,"==",record.atts[i]);
           // vector<int> v0 = Find_indices(index_name, "=", tb.attr_values[i]);
            if(!v0.empty())//if cover
            {
                cout<<"ERROR: Duplicate entry  for key 'Primary'"<<endl;
                return;
            }
        
        }
        else if(tb.attrs[i].attr_key_type == UNIQUE)
        {
            int offset = CTM.IndexOffset(tb.table_name, tb.attrs[i].attr_name);
            if(offset != -1)
            {
                Index idx = CTM.ReadIndex(tb.table_name, tb.attrs[i].attr_name, offset);
                
                vector<int> v0 = Select(idx.index_name,"==",record.atts[i]);
                // vector<int> v0 = Find_indices(index_name, "=", tb.attr_values[i]);
                if(!v0.empty())//if cover
                {
                    cout<<"ERROR: Duplicate entry  for key 'unique'"<<endl;
                    return;
                }
            }
        }
    }
    //insert successfully
    RCM.Insert(tb, record);
        cout<<"Query OK, 1 row affected\n"<<endl;
}



//inside function when operate on the log
void API_Select(string table_name, vector<string> & attr, string WClause)
{
    if(!CTM.TableExist(table_name))
    {
        cerr<<"ERROR:"<<"NO SUCH TABLE!"<<endl;
        return;
    }
    Table tb = CTM.ReadTable(table_name);
    Record record;
    int i = 0,count;
    int pos1 = 0,pos2;
    int end = 0;
    ConditionList Slist;
    
    vector<int> vd = Select(Slist[i].attr_name,Slist[i].cmp_value,Slist[i].cmp_value);
    sort(vd.begin(), vd.end());
    vd.erase(unique(vd.begin(), vd.end()),vd.end());
    vector<int>::iterator mIter;
    for(i = 1; i < Slist.size(); i++)
    {
        vector<int> vb = Select(Slist[i].attr_name,Slist[i].cmp_value,Slist[i].cmp_value);
        vector<int> va = vd;
        //vd = va jiao vb, va = vd
        sort(va.begin(), va.end());
        sort(vb.begin(), vb.end());
        vd.resize(va.size()+vb.size());
        mIter = set_intersection(va.begin(),va.end(), vb.begin(),vb.end(),vd.begin());
        vd.resize(mIter - vd.begin());
        //delete space
        sort(vd.begin(),vd.end());
        vd.erase(unique(vd.begin(),vd.end()),vd.end());
    }
    if(vd.empty())
    {
        cout<<"Empty Set"<<endl;
        return;
    }
    count = vd.size();
    API_DrawResult();
    cout<<count<<" rows in set"<<endl;
    
}

void API_Delete(string table_name,string WClasue)//undo
{
    //the same as select
    //create iterator
    ConditionList::iterator it;
    
    ConditionList WithIndexList,WithoutIndexList;
    Table tb = Read_Table_Info(table_name);
    //traversal condition list
    for(it = Dlist.begin(); it != Dlist.end(); it++)
    {
        //int ID = tb.searchAttrId(it->attr_name);
        if(Find_index_name(table_name, it->attr_name) == "")
            WithoutIndexList.push_back(*it);
        else
            WithIndexList.push_back(*it);
    }
    vector<int> va,vb,vd;
    vector<int>::iterator mIter;
    string index_name;
    
    //if no index
    if(WithIndexList.empty())
    {
        //extern string delete_values[32];
        for(int i = 0; i < 32;i ++)
            delete_values[i] = "";
        delete_num = 0;
        
        //delete record
        DeleteTuple(vd, tb, Dlist);
        
        /*********/
        cout<<"Query OK, "<<delete_num<<" rows affected"<<endl;
        
        if(select_num == 0)
            return;
        //遍历该表的attribute，找value，找index_name，delete
        for (int i = 0; i < tb.attr_num; i++)
        {
            //找条件列表中含索引项的属性的value和索引名
            //index_name
            string index_name = Find_index_name(table_name, tb.attrs[i].attr_name);
            //先确定属性有无索引, 无索引则直接进入下一循环
            if (index_name == "") {
                continue;
            }
            //取出delete_values的第i列，即为要B+树删除的value
            vector<string> vs = split(delete_values[i], "\t");
            //将该列的value逐一删除
            for (int j = 0; j < vs.size(); j++)
                DeleteIndex(index_name, vs[j]);
            
        }
        return;
    }
    it = WithIndexList.begin();
    index_name = Find_index_name(table_name, it->attr_name);
    vd = Find_indices(index_name, it->operation, it->cmp_value);
    
    //sort
    sort(vd.begin(), vd.end());
    vd.erase(unique(vd.begin(), vd.end()),vd.end());
    
    for(; it!= WithIndexList.end(); it++)
    {
        index_name = Find_index_name(table_name,it->attr_name);
        vb = Find_indices(index_name, it->operation, it->cmp_value);
        va = vb;
        
        //vd = va jiao vb, va = vd
        sort(va.begin(), va.end());
        sort(vb.begin(), vb.end());
        vd.resize(va.size()+vb.size());
        mIter = set_intersection(va.begin(),va.end(), vb.begin(),vb.end(),vd.begin());
        vd.resize(mIter - vd.begin());
        //delete space
        sort(vd.begin(),vd.end());
        vd.erase(unique(vd.begin(),vd.end()),vd.end());
    }
    
    if(vd.empty())
    {
        cout<<"Empty Set"<<endl;
        return;
    }
    
    for(int i = 0; i <32; i++)
        delete_num = 0;
    DeleteTuple(vd, tb, Dlist);
    cout<<"Query OK, "<<delete_num<<" rows affected"<<endl;
    if(delete_num == 0)
    {
        cout<<"Empty Set"<<endl;
        return;
    }
    for(int i = 0; i< tb.attr_num; i++)
    {
        string index_name = Find_index_name(table_name, tb.attrs[i].attr_name);
        if(index_name == "")
            continue;
        vector<string> vs = split(delete_values[i],"\t");
        for(int j = 0; j <vs.size(); j++)
            DeleteIndex(index_name, vs[j]);
    }
}

vector<string> split(string str, string pattern)
{
    string::size_type pos;
    vector<string> res;
    str += pattern
    int i;
    int size = str.size();
    for(i = 0; i< siez;i ++)
    {
        pos = str.find(pattern,i);
        if(pos < size)
        {
            string s = str.substr(i,pos-i);
            res.push_back(s);
            i = pos + pattern.size() - 1
            
        }
    }
    return res;
}

string split(string str,string pattern,int id)
{
    vector<string> res = split(str,parttern);
    return res[id];
}



void API_DrawResult(Table& tb,Record res)
{
    static int Width[32];
    int i;
    vector<string>::iterator it = select_values.begin();
    while(it != select_values.end())
    {
        vector<string> vs = split(*it, "\t");
        for(i = 0; i < tb.attr_num; i++)
        {
            if(vs[i].length() > Width[i])
                Width[i] = vs[i].length();
        }
        it ++;
    }
    
    for(i = 0; i < tb.attr_num; i++)
    {
        tb.attrs[i].attr_name.length() > Width[i]
            Width[i] = tb.attrs[i].attr_name.length();
    }
    for(i = 0; i < tb.attr_num; i++)
        Width[i] += 2;
    
    //draw
    
    
}

//merry the time
double Cal_time(long start, long end)
{
    return (double)(end - start)/CLK_TCK;
}




