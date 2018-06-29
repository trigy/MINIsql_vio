#include<iostream>
#include<string>
#include<vector>
#include<ctime>
#include"Minisql.h"
#include"API.h"

using namespace std;

string getInput()
{
    string SQLSentence, temp;
    char str[100];
    bool finish = false;
    while (!finish)
    {
        cin >> str;
        temp = str;
        SQLSentence = SQLSentence + " " + temp;
        if (SQLSentence[SQLSentence.size() - 1] == ';')
        {
            SQLSentence[SQLSentence.size() - 1] = ' ';
            SQLSentence += ";";
            finish = true;
        }
    }
    return SQLSentence;
}

string getInputFromFile(fstream &file)
{
    string SQLSentence, temp;
    char str[100];
    bool finish = false;
    while (!finish)
    {
        file >> str;
        temp = str;
        SQLSentence = SQLSentence + " " + temp;
        if (SQLSentence[SQLSentence.size() - 1] == ';')
        {
            SQLSentence[SQLSentence.size() - 1] = ' ';
            SQLSentence += ";";
            finish = true;
        }
    }
    return SQLSentence;
}

vector<string> GetTerm(string SQLSentence)
{
    string::iterator it;
    int start = 0;
    int count = 0;
    vector<string> term;
    for (int i = 0; i < SQLSentence.size(); i++)
    {
        if (SQLSentence[i] == '{' || SQLSentence[i] == '}' || SQLSentence[i] == '(' || SQLSentence[i] == ')' || SQLSentence[i] == ';' || SQLSentence[i] == ',')
        {
            if(count>0)
                term.push_back(SQLSentence.substr(start, count));
            term.push_back(SQLSentence.substr(i, 1));
            start=i+1;
            count=0;
        }
        else if(SQLSentence[i]=='='||SQLSentence[i]=='<'||SQLSentence[i]=='>')
        {
            if (count > 0)
                term.push_back(SQLSentence.substr(start, count));
            if (SQLSentence[i+1] == '=' || SQLSentence[i+1] == '<' || SQLSentence[i+1] == '>')
            {
                term.push_back(SQLSentence.substr(i, 2));
                i++;
                start = i + 1;
                count = 0;
            }
            else 
            {
                term.push_back(SQLSentence.substr(i, 1));
                start = i + 1;
                count = 0;
            }
        }
        else if (SQLSentence[i] == ' ')
        {
            if (start != i)
                term.push_back(SQLSentence.substr(start, count));
            start = i + 1;
            count = 0;
            }
            else
            {
                count++;
            }
    }
    return term;
}

int ReadPosInt(std::string a, int length)
{
    int result=0;
    for(int i=0;i<length;i++)
    {
        if(a[i]>='0'&&a[i]<='9')
        {
            result*=10;
            result+=a[i]-'0';
        }        
        else
        {
            return -1;
        }       
    }
    return result;
}

bool CheckFloat(std::string a)
{
    bool point=false;
    for(int i=0;i<a.size();i++)
    {
        if(a[i]=='.')
        {
            if(point)
            {
                return false;
            }
            else 
            {
                point=true;
            }
        }
        else if(a[i]>='0'&&a[i]<='9')
        {
            continue;
        }
        else 
        {
            return false;
        }
    }
    return true;
}

int ReadOperation(std::string op)
{
    if(op=="=")
    {
        return EQU;
    }
    else if(op=="<>")
    {
        return NEQ;
    }
    else if(op==">=")
    {
        return LGE;
    }
    else if(op=="<=")
    {
        return SME;
    }
    else if(op==">")
    {
        return LRG;
    }
    else if(op=="<")
    {
        return SML;
    }
    else 
    {
        return -1;
    }
}

bool GetAttrValue(string term, short type,char* dir)
{
    // cout<<"type: "<<type<<endl;
    if(type==INT)
    {
        if(term[0]=='-')
        {
            int re=ReadPosInt(term.substr(1),term.size()-1);
            if(re<0)
            {
                return false;
            }
            else 
            {
                re=-re;
                memcpy(dir, (char *)&re, 4);
                return true;
            }
        }
        else 
        {
            int re = ReadPosInt(term, term.size());
            if (re < 0)
            {
                return false;
            }
            else
            {
                memcpy(dir, (char *)&re, 4);
                return true;
            }
        }
    }
    else if(type==FLOAT)
    {
        if(CheckFloat(term))
        {
            float f;
            sscanf(term.data(),"%f",&f);
            memcpy(dir, (char *)&f, 4);
            return true;
        }
        else 
        {
            return false;
        }
    }
    else
    {
        if(term[0]=='\''&&term[type+1]=='\'')
        {
            memcpy(dir, term.data()+1, type);
            return true;
        }
        else 
        {
            return false;
        }
    }
}
int Interpreter(string input)
{
    vector<string> term = GetTerm(input);
    // for (int i = 0; i < term.size(); i++)
    // {
    //     cout << "[" << term[i] << "]";
    // }
    // cout << endl;
    if (term[0] == "create")
    {
        if (term[1] == "table")
        {
            Table table;
            table.table_name = term[2];
            table.attr_num = 0;
            if (term[3] == "(")
            {
                int i = 4;
                bool primaryFlag = false;
                while (term[i] != ")")
                {
                    for (int j = 0; j < table.attrs.size(); j++)
                    {
                        if (table.attrs[j].attr_name == term[i])
                        {
                            cerr << "ERROR: two attributes have the same name" << endl;
                            return -1;
                        }
                    }
                    if (term[i] == "primary" && term[i + 1] == "key" && term[i + 2] == "(" && term[i + 4] == ")")
                    {
                        if (primaryFlag)
                        {
                            cerr << "ERROR: more than one primary key" << endl;
                            return -1;
                        }
                        for (int j = 0; j < table.attrs.size(); j++)
                        {
                            if (table.attrs[j].attr_name == term[i + 3])
                            {
                                table.attrs[j].attr_key_type = PRIMARY;
                                primaryFlag = true;
                            }
                        }
                        if (!primaryFlag)
                        {
                            cerr << "ERROR: can't find the referenced attribute" << endl;
                            return -1;
                        }
                        i += 5;
                    }
                    else
                    {
                        Attribute attr;
                        attr.attr_name = term[i];
                        attr.attr_key_type = OTHER;
                        attr.attr_id = table.attrs.size();
                        if (term[i + 1] == "int")
                        {
                            attr.attr_type = INT;
                            i += 2;
                        }
                        else if (term[i + 1] == "float")
                        {
                            attr.attr_type = FLOAT;
                            i += 2;
                        }
                        else if (term[i + 1] == "char" && term[i + 2] == "(" && term[i + 4] == ")")
                        {
                            attr.attr_type = ReadPosInt(term[i + 3], term[i + 3].size());
                            if (attr.attr_type < 0)
                            {
                                cerr << "ERROR: stynax error near " << term[i + 3] << endl; //不是数字
                                return -1;
                            }
                            i += 5;
                        }
                        else
                        {
                            cerr << "ERROR: stynax error near " << term[i + 1] << ": invalid type" << endl; //错误的类型
                            return -1;
                        }
                        if (term[i] == "unique")
                        {
                            attr.attr_key_type = UNIQUE;
                            i++;
                        }
                        table.attrs.push_back(attr);
                        table.attr_num++;
                    }

                    if (term[i] == ")")
                        break;
                    else if (term[i] == "," && term[i] != ")")
                    {
                        i++;
                    }
                    else
                    {
                        cerr << "ERROR: stynax error near " << term[i - 1] << endl; //最后一行的多余逗号
                        return -1;
                    }
                }
                if (i > term.size())
                {
                    cerr << "ERROR: stynax error near " << term[term.size() - 2] << endl;
                    return -1;
                }
            }
            else
            {
                cerr << "ERROR: stynax error near " << term[2] << endl; //反正就是没到定义的时候
                return -1;
            }
            API_CreateTable(table);
            // cout<<"create table"<<endl;
        }
        else if (term[1] == "index")
        {
            if (term[3] == "on" && term[5] == "(" && term[7] == ")" && term[8] == ";")
            {
                Index index(term[2], term[4], term[6]);
                // API_CreateIndex(index);
            }
            else
            {
                cerr << "ERROR: stynax error near " << term[1] << endl;
                return -1;
            }
        }
    }
    else if (term[0] == "select")
    {
        vector<string> selectAttr;
        bool selectAttrAll = false;
        int i = 2;
        if (term[1] == "*")
        {
            selectAttrAll = true;
        }
        else
        {
            selectAttr.push_back(term[1]);
            while (term[i] != "from")
            {
                if (term[i] == ",")
                {
                    selectAttr.push_back(term[i + 1]);
                    i += 2;
                }
                else
                {
                    cerr << "ERROR: syntax error near " << term[i] << endl;
                    return -1;
                }
            }
        }
        i++;
        if (CTM.TableExist(term[i]))
        {
            Table table = CTM.ReadTable(term[i]);
            if (selectAttrAll)
            {
                for (int i = 0; i < table.attrs.size(); i++)
                {
                    selectAttr.push_back(table.attrs[i].attr_name);
                }
            }
            if (term[i + 1] == ";")
            {
                API_SelectAll(table, selectAttr);
            }
            else if (term[i + 1] == "where")
            {
                ConditionList cl;
                i += 2;
                Condition fcon;
                fcon.attr_name = term[i];
                fcon.operation = ReadOperation(term[i + 1]);
                if (fcon.operation < 0)
                {
                    cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                    return -1;
                }
                bool ffind = false;
                for (int j = 0; j < table.attrs.size(); j++)
                {
                    if (table.attrs[j].attr_name == fcon.attr_name)
                    {
                        if (GetAttrValue(term[i + 2], table.attrs[j].attr_type, fcon.cmp_value))
                        {
                            ffind = true;
                        }
                        else
                        {
                            cerr << "ERROR: Select type miss matched with Attribute " << table.attrs[j].attr_name << endl;
                        }
                    }
                }
                if (!ffind)
                {
                    cerr << "ERROR: Can not find Attribute " << fcon.attr_name << " in Table " << table.table_name << endl;
                }
                cl.push_back(fcon);
                i += 3;
                while (term[i] != ";")
                {
                    if (term[i] == "and")
                    {
                        i++;
                        Condition con;
                        con.attr_name = term[i];
                        con.operation = ReadOperation(term[i + 1]);
                        if (con.operation < 0)
                        {
                            cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                            return -1;
                        }
                        bool find = false;
                        for (int j = 0; j < table.attrs.size(); j++)
                        {
                            if (table.attrs[j].attr_name == con.attr_name)
                            {
                                if (GetAttrValue(term[i + 2], table.attrs[j].attr_type, con.cmp_value))
                                {
                                    find = true;
                                }
                                else
                                {
                                    cerr << "ERROR: Select type miss matched with Attribute " << table.attrs[j].attr_name << endl;
                                }
                            }
                        }
                        if (!find)
                        {
                            cerr << "ERROR: Can not find Attribute " << con.attr_name << " in Table " << table.table_name << endl;
                        }
                        cl.push_back(con);
                        i += 3;
                    }
                    else
                    {
                        cerr << "ERROR: stynax error near " << term[i] << endl;
                        return -1;
                    }
                }
                API_SelectCon(table, selectAttr, cl);
            }
        }
        else
        {
            cerr << "ERROR: table " << term[i] << "does not exist" << endl;
            return -1;
        }
    }
    else if (term[0] == "delete" && term[1] == "from")
    {
        int i = 2;
        if (CTM.TableExist(term[i]))
        {
            Table table = CTM.ReadTable(term[i]);
            if (term[i + 1] == ";")
            {
                API_DeleteAll(table);
            }
            else if (term[i + 1] == "where")
            {
                ConditionList cl;
                i += 2;
                Condition fcon;
                fcon.attr_name = term[i];
                fcon.operation = ReadOperation(term[i + 1]);
                if (fcon.operation < 0)
                {
                    cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                    return -1;
                }
                bool ffind = false;
                for (int j = 0; j < table.attrs.size(); j++)
                {
                    if (table.attrs[j].attr_name == fcon.attr_name)
                    {
                        if (GetAttrValue(term[i + 2], table.attrs[j].attr_type, fcon.cmp_value))
                        {
                            ffind = true;
                        }
                        else
                        {
                            cerr << "ERROR: Select type miss matched with Attribute " << table.attrs[j].attr_name << endl;
                        }
                    }
                }
                if (!ffind)
                {
                    cerr << "ERROR: Can not find Attribute " << fcon.attr_name << " in Table " << table.table_name << endl;
                }
                cl.push_back(fcon);
                i += 3;
                while (term[i] != ";")
                {
                    if (term[i] == "and")
                    {
                        i++;
                        Condition con;
                        con.attr_name = term[i];
                        con.operation = ReadOperation(term[i + 1]);
                        if (con.operation < 0)
                        {
                            cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                            return -1;
                        }
                        bool find = false;
                        for (int j = 0; j < table.attrs.size(); j++)
                        {
                            if (table.attrs[j].attr_name == con.attr_name)
                            {
                                if (GetAttrValue(term[i + 2], table.attrs[j].attr_type, con.cmp_value))
                                {
                                    find = true;
                                }
                                else
                                {
                                    cerr << "ERROR: Select type miss matched with Attribute " << table.attrs[j].attr_name << endl;
                                }
                            }
                        }
                        if (!find)
                        {
                            cerr << "ERROR: Can not find Attribute " << con.attr_name << " in Table " << table.table_name << endl;
                        }
                        cl.push_back(con);
                        i += 3;
                    }
                    else
                    {
                        cerr << "ERROR: stynax error near " << term[i] << endl;
                        return -1;
                    }
                }
                API_DeleteCon(table, cl);
            }
        }
        else
        {
            cerr << "ERROR: table does not exist" << endl;
            return -1;
        }
    }
    else if (term[0] == "insert" && term[1] == "into")
    {
        if (!CTM.TableExist(term[2]))
        {
            cerr << "ERROR: table " << term[2] << " does not exist" << endl;
            return -1;
        }
        Table table = CTM.ReadTable(term[2]);
        // cout<<"table attr no: "<<table.attrs.size()<<endl;
        int valueCount = 0;
        if (term[3] == "values" && term[4] == "(")
        {
            Record record;
            int i = 5;
            if (valueCount == table.attrs.size())
            {
                cerr << "ERROR: Insert values is miss matched with Table " << table.table_name << endl;
                return -1;
            }
            if (term[i] == "NULL")
            {
                record.null.push_back(true);
            }
            else
            {
                record.null.push_back(false);
                if (!GetAttrValue(term[i], table.attrs[valueCount].attr_type, record.atts[valueCount]))
                {
                    cerr << "ERROR: Insert values type error near " << term[i] << endl;
                    return -1;
                }
            }
            valueCount++;
            i++;
            while (term[i] != ")")
            {
                if (term[i] == ",")
                {
                    i++;
                    if (valueCount == table.attrs.size())
                    {
                        cerr << "ERROR: Insert values is miss matched with Table " << table.table_name << endl;
                        return -1;
                    }
                    if (term[i] == "NULL")
                    {
                        record.null.push_back(true);
                    }
                    else
                    {
                        record.null.push_back(false);
                        if (!GetAttrValue(term[i], table.attrs[valueCount].attr_type, record.atts[valueCount]))
                        {
                            cerr << "ERROR: Insert values type error near " << term[i] << endl;
                            return -1;
                        }
                    }
                    valueCount++;
                    i++;
                }
                else
                {
                    cerr << "ERROR: stynax error near " << term[i] << endl;
                    return -1;
                }
                if (i > term.size())
                {
                    cerr << "ERROR: stynax error near " << term[term.size() - 2] << endl;
                    return -1;
                }
            }
            if (term[i + 1] == ";")
            {
                API_Insert(table, record);
            }
            else
            {
                cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                return -1;
            }
        }
        else
        {
            cerr << "ERROR: stynax error near " << term[2] << endl;
            return -1;
        }
    }
    else if (term[0] == "drop")
    {
        if (term[1] == "table" && term[3] == ";")
        {
            API_DropTable(term[2]);
        }
        else if (term[1] == "index" && term[3] == "on" && term[5] == ";") //drop index xxx on xxx;
        {
            API_DropIndex(term[2], term[4]);
        }
    }
    else if (term[0] == "execfile")
    {
        if (term[2] == ";")
        {
            fstream file;
            file.open(term[1], ios::in);
            if (!file.good())
            {
                cerr << "ERROR: can not find file " << term[1] << endl;
                return -1;
            }
        while(1)
        {
            if(!file.good())
            {
                return 0;
            }
            string fi=getInputFromFile(file);
            int result=Interpreter(fi);
            if(result<0)
            {
                return result;
            }
        }
        }
        else
        {
            cerr << "ERROR: stynax error near " << term[2] << endl;
            return -1;
        }
    }
    else if (term[0] == "quit")
    {
        if (term[1] == ";")
        {
            // bf.WriteBackAllDirtyBlock();
            return -2;
        }
        else
        {
            cerr << "ERROR: stynax error near " << term[1] << endl;
            return -1;
        }
    }
    else
    {
        cerr << "ERROR: stynax error near " << term[0] << endl;
        return -1;
    }
    return 0;
}

int main()
{
    // RecordManager RCM;
    // CatalogManager CTM;
    // IndexManager IDM;
    // string a="create table student{id int, name char(8)};";
    // string a="2155  4244 53333";
    while(1)
    {
        cout<<"minisql> ";
        string a=getInput();
        clock_t start=clock();
        if(Interpreter(a)==-2)
        {
            break;
        }
        clock_t end=clock();
        cout<<"takes "<<((double)(end-start)/CLOCKS_PER_SEC)<<" second(s)"<<endl;
    }
    bf.WriteBackAllDirtyBlock();
}