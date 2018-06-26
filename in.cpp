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

int main()
{
    // RecordManager RCM;
    // CatalogManager CTM;
    // IndexManager IDM;
    // string a=getInput();
    string a="create table student{id int};";
    // string a="2155  4244 53333";
    vector<string> term=GetTerm(a);
    for(int i=0;i<term.size();i++)
    {
        cout<<"["<<term[i]<<"]";
    }
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
                while(term[i]=="}")
                {
                    for (int j = 0; j < table.attrs.size(); j++)
                    {
                        if (table.attrs[j].attr_name == term[i])
                        {
                            cerr<<"two attributes have the same name"<<endl;
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
                            cerr<<"more than one primary key"<<endl;
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
                            cerr<<"can't find the referenced attribute"<<endl;
                            errorFlag = true;
                            break;
                        }
                    }
                    else 
                    {
                        if(term[i+1]=="int")
                        {
                            attr.attr_type=INT;
                            i++;
                        }
                        else if(term[i+1]=="float")
                        {
                            attr.attr_type=FLOAT;
                            i++;
                        }
                        else if(term[i+1]=="char"&&term[i+2]=="("&&term[i+4]==")")
                        {                      
                            attr.attr_type=ReadPosInt(term[i+3],term[i+3].size());
                            if(attr.attr_type<0) 
                            {
                                cerr<<"SE"<<endl; //不是数字
                                break;
                            }
                            i+=4;
                        }
                        else 
                        {
                            cerr << "SE" << endl; //错误的类型
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
                        cerr << "SE" << endl; //最后一行的多余逗号
                        errorFlag = true;
                        break;
                    }
                }

            }
            else 
            {
                cerr<<"stynax error"<<endl; //反正就是没到定义的时候
                errorFlag = true;
            }
            if(!errorFlag)
            {
                API_CreateTable(table);
            }
        }
        else if(term[1]=="index")
        {
            if(term[3]=="on"&&term[5]=="("&&term[7]==")"&&term[8]==";")
            {
                Index index(term[2],term[4],term[6]);
                API_CreateIndex(index);
            }
            else 
            {
                cerr<<"stynax error"<<endl;
            }
        }
    }
    else if(term[0]=="select")
    {

    }
    else if(term[0]=="delete")
    {

    }
    else if(term[0]=="insert")
    {

    }
    else if(term[0]=="drop")
    {

    }
    else if(term[0]=="execfile")
    {

    }
    else if(term[0]=="quit")
    {
        if(term[1]==";")
        {
            bf.WriteBackAllDirtyBlock();
        }
    }
    bf.WriteBackAllDirtyBlock();
}