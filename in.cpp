#include<iostream>
#include<string>
#include<vector>
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
        if(a[i]>'0'&&a[i]<'9')
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

int main()
{
    // RecordManager RCM;
    // CatalogManager CTM;
    // IndexManager IDM;
    // string a=getInput();
    string a="create table student{id int, name char(8)};";
    // string a="2155  4244 53333";
    vector<string> term=GetTerm(a);
    for(int i=0;i<term.size();i++)
    {
        cout<<"["<<term[i]<<"]";
    }
    cout<<endl;
    bool errorFlag=false;
    if(term[0]=="create")
    {
        if(term[1]=="table")
        {
            Table table;
            table.table_name=term[2];
            table.attr_num=0;
            if(term[3]=="{")
            {
                int i=4;
                bool primaryFlag=false;
                while(term[i]!="}")
                {
                    for (int j = 0; j < table.attrs.size(); j++)
                    {
                        if (table.attrs[j].attr_name == term[i])
                        {
                            cerr<<"ERROR: two attributes have the same name"<<endl;
                            errorFlag=true;
                            break;
                        }
                    }
                    Attribute attr;
                    attr.attr_name=term[i];
                    attr.attr_key_type=OTHER;
                    attr.attr_id=table.attrs.size();
                    if(term[i+1]=="primary"&&term[i+2]=="key"&&term[i+3]=="("&&term[i+5]==")")
                    {
                        if(primaryFlag)
                        {
                            cerr<<"ERROR: more than one primary key"<<endl;
                            errorFlag = true;
                            break;
                        }
                        for(int j=0;j<table.attrs.size();j++)
                        {
                            if(table.attrs[j].attr_name==term[i+4])
                            {
                                table.attrs[j].attr_key_type=PRIMARY;
                                primaryFlag=true;
                            }
                        }
                        if(!primaryFlag)
                        {
                            cerr<<"ERROR: can't find the referenced attribute"<<endl;
                            errorFlag = true;
                            break;
                        }
                    }
                    else 
                    {
                        if(term[i+1]=="int")
                        {
                            attr.attr_type=INT;
                            i+=2;
                        }
                        else if(term[i+1]=="float")
                        {
                            attr.attr_type=FLOAT;
                            i+=2;
                        }
                        else if(term[i+1]=="char"&&term[i+2]=="("&&term[i+4]==")")
                        {                      
                            attr.attr_type=ReadPosInt(term[i+3],term[i+3].size());
                            if(attr.attr_type<0) 
                            {
                                cerr << "ERROR: stynax error near " << term[i+3] << endl; //不是数字
                                break;
                            }
                            i+=5;
                        }
                        else 
                        {
                            cerr << "ERROR: stynax error near " << term[i+1] << endl; //错误的类型
                            break;
                        }
                        if(term[i+1]=="unique")
                        {
                            attr.attr_key_type=UNIQUE;
                            i++;
                        }
                        table.attrs.push_back(attr);
                        table.attr_num++;
                    }

                    if(term[i]=="}") break;
                    else if(term[i]==","&&term[i]!="}")
                    {
                        i++;
                    }
                    else 
                    {
                        cerr << "ERROR: stynax error near " << term[i-1] << endl; //最后一行的多余逗号
                        errorFlag = true;
                        break;
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
                cerr<<"ERROR: stynax error near "<<term[2]<<endl; //反正就是没到定义的时候
                errorFlag = true;
            }
            if(!errorFlag)
            {
                // API_CreateTable(table);
                cout<<"create table"<<endl;
            }
        }
        else if(term[1]=="index")
        {
            if(term[3]=="on"&&term[5]=="("&&term[7]==")"&&term[8]==";")
            {
                Index index(term[2],term[4],term[6]);
                // API_CreateIndex(index);
            }
            else 
            {
                cerr << "ERROR: stynax error near " << term[1] << endl;
            }
        }
    }
    else if(term[0]=="select")
    {
        vector<string> selectAttr;
        bool selectAll=false;
        int i = 2;
        if(term[1]=="*")
        {
            selectAll=true;
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
            }
        }   
        if(CTM.TableExist(term[i]))
        {
            Table table=CTM.ReadTable(term[i]);
            if(term[i+1]==";")
            {
                API_SelectAll(table,selectAttr);
            }
            else if(term[i+1]=="where")
            {
                ConditionList cl;
                i+=2;
                Condition fcon;
                fcon.attr_name=term[i];
                fcon.operation=ReadOperation(term[i+1]);
                fcon.cmp_value=term[i+2];
                if(fcon.operation<0)
                {
                    cerr << "ERROR: stynax error near " << term[i+1] << endl;
                    return -1;
                }
                cl.push_back(fcon);
                i+=3;
                while(term[i]!=";")
                {
                    if(term[i]=="and")
                    {
                        i++;
                        Condition con;
                        con.attr_name = term[i];
                        con.operation = ReadOperation(term[i + 1]);
                        con.cmp_value = term[i + 2];
                        if (con.operation < 0)
                        {
                            cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                            return -1;
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
                API_SelectCon(table,selectAttr,cl);
            }
        }
        else 
        {
            cerr<<"ERROR: table does not exist"<<endl;
        }
    }
    else if(term[0]=="delete" && term[1]=="from")
    {
        int i=2;
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
                fcon.cmp_value = term[i + 2];
                if (fcon.operation < 0)
                {
                    cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                    return 0;
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
                        con.cmp_value = term[i + 2];
                        if (con.operation < 0)
                        {
                            cerr << "ERROR: stynax error near " << term[i + 1] << endl;
                            return -1;
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
        }
    }
    else if(term[0]=="insert" && term[1]=="into")
    {
        Table table=CTM.ReadTable(term[2]);
        if(term[3]=="values" && term[4]=="(")
        {
            int i=5;
            Record record;
            record.atts.push_back(term[i]);
            if(term[i]=="NULL")
                record.null.push_back(true);
            else 
                record.null.push_back(false);
            i++;
            while(term[i]!=")")
            {
                if(term[i]==",")
                {
                    i++;
                    record.atts.push_back(term[i]);
                    if (term[i] == "NULL")
                        record.null.push_back(true);
                    else
                        record.null.push_back(false);
                    i++;
                }
                else 
                {
                    cerr << "ERROR: stynax error near " << term[i] << endl;
                }
                if(i>term.size())
                {
                    cerr << "ERROR: stynax error near " << term[term.size()-2] << endl;
                    return -1;
                }
            }
            if(term[i+1]==";")
            {
                API_Insert(table,record);
            }
            else 
            {
                cerr << "ERROR: stynax error near " << term[i+1] << endl;
                return -1;
            }
        }
    }
    else if(term[0]=="drop")
    {
        if(term[1]=="table" && term[3]==";")
        {
            API_DropTable(term[2]);
        }
        else if(term[1]=="index" && term[3]=="on" && term[5]==";") //drop index xxx on xxx;
        {
            API_DropIndex(term[2],term[4]);
        }
    }
    else if(term[0]=="execfile")
    {
        
    }
    else if(term[0]=="quit")
    {
        if(term[1]==";")
        {
            // bf.WriteBackAllDirtyBlock();
        } 
        else 
        {
            cerr<< "ERROR: stynax error near " << term[0] <<endl;
        }
    } 
    else 
    {
       cerr<< "ERROR: stynax error near " << term[0] <<endl;
    }
    // bf.WriteBackAllDirtyBlock();
}