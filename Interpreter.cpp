//
//  Interpreter.cpp
//  Minisql
//
//  Created by zhe on 2018/6/21.
//  Copyright © 2018年 zhezhezhe. All rights reserved.
//

#include "Interpreter.hpp"
#include "API.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string.h>
#include <fstream>
#define NULLPTR nullptr
using namespace std;

static char address[1000];
static ifstream ifs;

/******************************************************************/
//get information
string getInput()
{
    string SQLSentence,temp;
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

/******************************************************************/
/////analys the select clause then change the object of condition class
int select_clause(string &SQLSentence,  int &SQLCurrentPointer, int &end, condition &SQLCondition)
{
    string CurWord;
    SQLCondition.setInstruction(SELECT);
    
    //step2
    if ((end = SQLSentence.find(" from", SQLCurrentPointer)) == -1)//find " from"
    {
        cerr << "ERROR: " << "Can not find key words \"from\"." << endl;//can not find from
        end = SQLSentence.find(";", SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    
    //step3
    while (SQLSentence[end] == ' ')//select xxx from ooo, this is to find where is the end of xxx
        --end;
    while (SQLSentence[SQLCurrentPointer] == ' ' && end >= SQLCurrentPointer)//select xxx from ooo, this is to find where is the front of xxx
        ++SQLCurrentPointer;
    end++;//adjust the pointer to the space right after xxx
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
    
    //step4
    if (CurWord.empty() == 1)//can not find col name
    {
        cerr << "ERROR: " << "Can not find col name." << endl;
        end = SQLSentence.find(";", SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    
    //step5
    if (CurWord != "*")//not select all col name
    {
        int f = 0, e = 0;
        string col;
        while (e < CurWord.size())
        {
            if (CurWord[e] == ' ')
            {
                int flag = 0;
                if (CurWord[e - 1] != ',')
                    flag = 1;
                while (CurWord[e] == ' ' && e < CurWord.size())
                    ++e;
                if (flag && CurWord[e] != ',')
                {
                    cerr << "Error: " << "Col name must be separated by \',\'." << endl;
                    end = SQLSentence.find(";", SQLCurrentPointer);
                    SQLCurrentPointer = end;
                    return ERROR;
                }
                else if (CurWord[e] == ',')
                    f = ++e;
                else
                    f = e;
            }
            while (CurWord[e] != ',' && CurWord[e] != ' ' && e < CurWord.size())
                ++e;
            col = CurWord.substr(f, e - f);
            SQLCondition.setColName(col);
            f = e + 1;//move to next col
            if (CurWord[e] == ',')
                ++e;
            if (CurWord[e] == ' ')
            {
                int flag = 0;
                if (CurWord[e - 1] != ',')
                    flag = 1;
                while (CurWord[e] == ' ' && e <= CurWord.size())
                    ++e;
                if (e + 1 == CurWord.size() && flag == 1)
                {
                    cerr << "ERROR: " << "Col name has syntax error." << endl;
                    end = SQLSentence.find(";", SQLCurrentPointer);
                    SQLCurrentPointer = end;
                    return ERROR;
                }
                if (flag && CurWord[e] != ',')
                {
                    cerr << "ERROR:" << "Col name must be separated by \',\'." << endl;
                    end = SQLSentence.find(";", SQLCurrentPointer);
                    SQLCurrentPointer = end;
                    return ERROR;
                }
                else if (CurWord[e] == ',')
                    f = ++e;
                else
                    f = e;
            }
        }
        
    }
    else
    {
        SQLCondition.setColName(CurWord);
    }
    
    //step6
    while (SQLSentence[end] == ' ')//select xxx ,this is to find where is the front of from
        ++end;
    SQLCurrentPointer = end;
    
    //step7
    end = SQLSentence.find(' ', SQLCurrentPointer);
    if (end == -1)
    {
        cerr << "ERROR:" << "Can not find table name." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    SQLCurrentPointer = end;
    
    //step8
    while (SQLSentence[end] == ' ')//from ooo, this is to find where is the front of ooo
        ++end;
    SQLCurrentPointer = end;
    while (SQLSentence[end] != ';' && SQLSentence[end] != ' ')
        ++end;
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
    
    //step9
    if (CurWord.empty() || CurWord == "where")
    {
        cerr << "ERROR:" << "The table name can not be found" << endl;
        end = SQLSentence.find(";", SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    
    //step10
    SQLCondition.setTableName(CurWord);
    //step11
    while (SQLSentence[end] == ' ')
        ++end;
    SQLCurrentPointer = end;
    if (SQLSentence[end] == ';')
        return SELECT;
    else
    {
        SQLCurrentPointer = SQLSentence.find("where", 0);
        while (SQLSentence[SQLCurrentPointer] != ' ' && SQLSentence[SQLCurrentPointer] != ';')
            ++SQLCurrentPointer;
        end = SQLSentence.find(';', 0);
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
        SQLCondition.setWhereClause(CurWord);
        return SELECT;
    }
}

/////////////////////////////////////////////////
/////Function No. 3:
/////analusis the drop clause then change the object of condition class
int drop_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition)
{
    string CurWord;
    
    //step1
    end = SQLSentence.find("table", SQLCurrentPointer);
    if (end != -1)//drop table
    {
        SQLCondition.setInstruction(DROP_TABLE);
        SQLCurrentPointer = end;//move pointer to 't'("drop table oooo")
        while (SQLSentence[end] != ' ' && end < SQLSentence.size())//move across "table"
            ++end;
        while (SQLSentence[end] == ' ' && end < SQLSentence.size())// get rid of ' '
            ++end;
        if (SQLSentence[end] == ';')//lose table name
        {
            cerr << "ERROR:" << "Can not find table name." << endl;
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCurrentPointer = end;
        end = SQLSentence.find(';', SQLCurrentPointer);
        if(SQLSentence[end - 1] == ' ')
            --end;
        while (SQLSentence[end] == ' ' && end > SQLCurrentPointer)// get rid of ' '
            --end;
        if (SQLSentence[end] == ';')
            CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);//get table name
        else
            CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);//get table name
        
        //set tableName
        SQLCondition.setTableName(CurWord);
        SQLCurrentPointer = end;
    }
    else//drop index
    {
        end = SQLSentence.find("index", SQLCurrentPointer);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key word 'table' or 'index'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCondition.setInstruction(DROP_INDEX);
        ////
        end = SQLSentence.find(" on", SQLCurrentPointer);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key word /'on'/." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCurrentPointer++;
        while (SQLSentence[SQLCurrentPointer] != ' ')//across index
            SQLCurrentPointer++;
        while (SQLSentence[SQLCurrentPointer] == ' ')
            SQLCurrentPointer++;
        while (end > SQLCurrentPointer && SQLSentence[end] == ' ')
            --end;
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        
        if (CurWord.empty())
        {
            cerr << "ERROR:" << "Can not find index name." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        SQLCondition.setIndexName(CurWord);
        end = SQLSentence.find("on", SQLCurrentPointer);
        SQLCurrentPointer = end;
        while (SQLSentence[SQLCurrentPointer] != ' ')//across on
            SQLCurrentPointer++;
        while (SQLSentence[SQLCurrentPointer] == ' ')
            SQLCurrentPointer++;
        end = SQLSentence.find(";", SQLCurrentPointer);
        --end;
        while (end > SQLCurrentPointer && SQLSentence[end] == ' ')
            --end;
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        SQLCondition.setTableName(CurWord);
        SQLCurrentPointer = ++end;
    }
    return DROP;
}

/////////////////////////////////////////////////
/////Function No. 3:
/////analusis the insert clause then change the object of condition class
int insert_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition)
{
    string CurWord;
    //step0
    SQLCondition.setInstruction(INSERT);
    
    //step1
    while (SQLSentence[SQLCurrentPointer] == ' ')//move to next word
        ++SQLCurrentPointer;
    
    //step2
    if (SQLSentence[SQLCurrentPointer] == ';')
    {
        cerr << "ERROR:" << "Can not find key word 'into'." << endl;
        end = SQLCurrentPointer;
        return ERROR;
    }
    
    //step3
    end = SQLCurrentPointer;
    while (SQLSentence[end] != ' ' && SQLSentence[end] != ';')
        ++end;
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
    if (CurWord != "into")
    {
        cerr << "ERROR:" << "Can not find key word 'into'." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    
    //step4
    SQLCurrentPointer = end;
    end = SQLSentence.find(" values", SQLCurrentPointer);
    if (end == -1)
    {
        cerr << "ERROR:" << "Can not find key word 'values'." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    while (SQLSentence[end] == ' ' && end > SQLCurrentPointer)
        --end;
    end += 1;
    while (SQLSentence[SQLCurrentPointer] == ' ' && end > SQLCurrentPointer)
        ++SQLCurrentPointer;
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
    
    if(CurWord.empty())
    {
        cerr << "ERROR:" << "Can not find table name." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    SQLCondition.setTableName(CurWord);
    
    //step5
    end = SQLSentence.find("values", SQLCurrentPointer);
    while (SQLSentence[end] != ' ' && SQLSentence[end] != ';' && SQLSentence[end] != '(')
        ++end;
    if (SQLSentence[end] == ';')
    {
        cerr << "ERROR:" << "Can not find insert values." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    
    //step6
    SQLCurrentPointer = SQLSentence.find('(');
    if (SQLCurrentPointer == -1)
    {
        cerr << "ERROR:" << "Can not find '('." << endl;
        end = SQLSentence.find(';', 0);
        SQLCurrentPointer = end;
        return ERROR;
    }
    end = SQLSentence.find(')');
    if (end == -1)
    {
        cerr << "ERROR:" << "Can not find ')'." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    end = SQLSentence.find(';');
    while (SQLSentence[end] != ')')
        --end;
    if (SQLSentence[SQLCurrentPointer + 1] == '(')
    {
        cerr << "ERROR:" << "The InsertValues is empty." << endl;
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;
        return ERROR;
    }
    ++SQLCurrentPointer;
    --end;
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
    SQLCondition.setInsertValues(CurWord);
    //step7,end
    end = SQLSentence.find(';', SQLCurrentPointer);
    SQLCurrentPointer = end;
    return INSERT;
}

/////////////////////////////////////////////////
/////Function No. 4:
/////analusis the create clause then change the object of condition class
int create_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition)
{
    string CurWord;
    
    //step1
    end = SQLSentence.find("table", SQLCurrentPointer);
    
    if (end != -1) //"table" exists
    {
        SQLCondition.setInstruction(CREATE_TABLE);
        //step2.a
        SQLCurrentPointer = end; // adjust the pointer to 't' in table
        while (SQLSentence[SQLCurrentPointer] != ' ' && SQLSentence[SQLCurrentPointer] != ';' && SQLCurrentPointer <= SQLSentence.size())
            ++SQLCurrentPointer;//across "table"
        end = SQLSentence.find('(', 0);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key symbol '('." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step3.a
        --end;//adjust to the space right before the '('
        while (SQLSentence[end] == ' ' && SQLCurrentPointer < end)
            --end;
        if (SQLCurrentPointer == end)
        {
            cerr << "ERROR:" << "Can not find table name." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step4.a
        while (SQLSentence[SQLCurrentPointer] == ' ' && SQLCurrentPointer < end)
            ++SQLCurrentPointer;
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        int i = 0;
        while (CurWord[i] != ' ' && i < CurWord.size())
            ++i;
        if (i != CurWord.size())
        {
            cerr << "ERROR:" << "Find a sytnax fault between \"table\" and '('." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCondition.setTableName(CurWord);
        
        //step5.a
        SQLCurrentPointer = SQLSentence.find('(', 0);
        end = SQLSentence.find(';', SQLCurrentPointer);
        while (SQLSentence[end] != ')')
            --end;
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key symbol ')'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step6.a
        if (end == SQLCurrentPointer + 1)
        {
            cerr << "ERROR:" << "Can not find attribute." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step7.a
        ++SQLCurrentPointer;
        --end;
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        SQLCondition.setAttribute(CurWord);
        return CREATE_TABLE;
    }
    else//create "index"?
    {
        //step2.b
        end = SQLSentence.find("index", 0);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key word 'table' or 'index'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        SQLCondition.setInstruction(CREATE_INDEX);
        //step3.b
        end = SQLSentence.find("on", 0);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key word 'on'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step4.b
        SQLCurrentPointer = SQLSentence.find("index", 0);
        while (SQLSentence[SQLCurrentPointer] != ' ' && SQLSentence[SQLCurrentPointer] != ';' && SQLCurrentPointer < SQLSentence.size())
            ++SQLCurrentPointer;//across index
        
        //step5.b
        --end;
        while (SQLSentence[end] == ' ' && SQLCurrentPointer < end)
            --end;
        while (SQLSentence[SQLCurrentPointer] == ' ' && SQLCurrentPointer < end)
            ++SQLCurrentPointer;
        if (end == SQLCurrentPointer)
        {
            cerr << "ERROR:" << "Can not find index name." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step6.b
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        int i = 0;
        while (CurWord[i] != ' ' && i < CurWord.size())
            ++i;
        if (i != CurWord.size())
        {
            cerr << "ERROR:" << "Find a sytnax fault between \'index\" and \"on\"." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCondition.setIndexName(CurWord);
        
        //step7.b
        end = SQLSentence.find("on", 0);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key word 'on'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step8.b
        SQLCurrentPointer = end;
        while (SQLSentence[SQLCurrentPointer] == 'o' || SQLSentence[SQLCurrentPointer] == 'n')
            ++SQLCurrentPointer;//across 'on'
        
        //step9.b
        end = SQLSentence.find('(', 0);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key symbol '('." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step10.b
        --end;
        while (SQLSentence[end] == ' ' && SQLCurrentPointer < end)
            --end;
        while (SQLSentence[SQLCurrentPointer] == ' ' && SQLCurrentPointer < end)
            ++SQLCurrentPointer;
        if (end == SQLCurrentPointer)
        {
            cerr << "ERROR:" << "Can not find table name." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step11.b
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        i = 0;
        while (CurWord[i] != ' ' && i < CurWord.size())
            ++i;
        if (i != CurWord.size())
        {
            cerr << "ERROR:" << "Find a sytnax fault between \"on\" and '('." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCondition.setTableName(CurWord);
        
        //step12.b
        SQLCurrentPointer = SQLSentence.find('(', 0);
        end = SQLSentence.find(')', SQLCurrentPointer);
        if (end == -1)
        {
            cerr << "ERROR:" << "Can not find key symbol '）'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step14.b
        if (end == SQLCurrentPointer + 1)
        {
            cerr << "ERROR:" << "Can not find attribute." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        
        //step7.a
        ++SQLCurrentPointer;
        --end;
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        SQLCondition.setAttribute(CurWord);
        return CREATE_INDEX;
    }
}

/////////////////////////////////////////////////
/////Function No. 5:
/////analusis the delect clause then change the object of condition class
int delect_clauese(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition)
{
    string CurWord;
    SQLCondition.setInstruction(DELETE);
    //step1
    if (SQLSentence.find("delete from", 0) == -1)//error
    {
        if (SQLSentence.find("from", 0) == -1)//can not find from
        {
            cerr << "ERROR:" << "Can not find key word 'from'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        else
        {
            cerr << "ERROR:" << "There are some syntax error between \"delete\" and \"from\"." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
    }
    
    //step2
    SQLCurrentPointer = SQLSentence.find("from", 0);
    
    //step3
    while (SQLSentence[SQLCurrentPointer] != ' ' && SQLSentence[SQLCurrentPointer] != ';')
        ++SQLCurrentPointer;//now pointer is point to "from "'s end space
    
    //step4
    end = SQLSentence.find("where", 0);
    
    if (end == -1)
    {
        end = SQLSentence.find(';', 0);
        --end;
        while (SQLSentence[end] == ' ' && end > SQLCurrentPointer)
            --end;
        while (SQLSentence[SQLCurrentPointer] == ' ' && end > SQLCurrentPointer)
            ++SQLCurrentPointer;
        if (SQLCurrentPointer == end)
        {
            cerr << "ERROR:" << "Can not find table name." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
        int i = 0;
        while (CurWord[i] != ' ' && i < CurWord.size())
            ++i;
        if (i != CurWord.size())
        {
            cerr << "ERROR:" << "Find a sytnax fault between \"from\" and ';'." << endl;
            end = SQLSentence.find(';', SQLCurrentPointer);
            SQLCurrentPointer = end;
            return ERROR;
        }
        SQLCondition.setTableName(CurWord);
        SQLCondition.setWhereClause("");
        end = SQLSentence.find(';', SQLCurrentPointer);
        SQLCurrentPointer = end;//end
        return DELETE;
    }
    
    --end;
    while (SQLSentence[end] == ' ' && end > SQLCurrentPointer)
        --end;
    while (SQLSentence[SQLCurrentPointer] == ' ' && end > SQLCurrentPointer)
        ++SQLCurrentPointer;
    if (SQLCurrentPointer == end)
    {
        cerr << "ERROR:" << "Can not find table name." << endl;
        end = SQLSentence.find(';', 0);
        SQLCurrentPointer = end;
        return ERROR;
    }
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer + 1);
    int i = 0;
    while (CurWord[i] != ' ' && i < CurWord.size())
        ++i;
    if (i != CurWord.size())
    {
        cerr << "ERROR:" << "Find a sytnax fault between \"from\" and 'where'." << endl;
        end = SQLSentence.find(';', 0);
        SQLCurrentPointer = end;
        return ERROR;
    }
    SQLCondition.setTableName(CurWord);
    
    //find where clause
    SQLCurrentPointer = SQLSentence.find("where", 0);
    while (SQLSentence[SQLCurrentPointer] != ' ' && SQLSentence[SQLCurrentPointer] != ';')
        ++SQLCurrentPointer;
    end = SQLSentence.find(';', 0);
    CurWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);
    SQLCondition.setWhereClause(CurWord);
    
    
    //end
    end = SQLSentence.find(';', SQLCurrentPointer);
    SQLCurrentPointer = end;
    return DELETE;
}


extern block blocks[BLOCKNUMBER];
//处理每一行的attribute
int attrDef(Attribute Att ,string attDef)//处理表的属性定义
{
    attDef+=' ';
    int pos1,pos2;//考虑多个空格
    pos1 = 0;
    
    //处理空格和格式错误
    while (pos1<=attDef.length()-1&&(attDef[pos1]==' '||attDef[pos1]=='\n')) pos1++;
    if ((pos1>attDef.length()-1)||(attDef.find(' ',pos1)==-1&&attDef.find('\n',pos1)==-1))
    {
        cerr << "ERROR:" << "Wrong Format"<<endl;
        return -1;
    }
    
    pos2 = min(attDef.find(' ',pos1),attDef.find('\n',pos1));
    Att.attr_name=attDef.substr(pos1,pos2-pos1);//第一段有意义的字符串是attributename
    
    pos1 = pos2+1;
    //处理空格和格式错误
    while (pos1<=attDef.length()-1&&(attDef[pos1]==' '||attDef[pos1]=='\n')) pos1++;
    if ( (pos1>attDef.length()-1)||(attDef.find(' ',pos1)==-1&&attDef.find('\n',pos1)==-1)) //报错
    {
        cerr << "ERROR:" << "Wrong Format"<<endl;
        return -1;
    }
    
    pos2 = min(attDef.find(' ',pos1),attDef.find('\n',pos1));
    //第二个字符串是类型
    if (attDef.substr(pos1,3)=="int")
        Att.attr_type = INT;
    else
        if (attDef.substr(pos1,5)=="float")
            Att.attr_type = FLOAT;
        else
            if (attDef.substr(pos1,4)=="char")
            {
                int pos1 = attDef.find('(');
                int pos2 = attDef.find(')');
                Att.attr_type = atoi(attDef.substr(pos1+1,pos2-pos1-1).c_str());
            }
            else
            {
                //不属于三种类型，错误
                cerr << "ERROR:" << "Wrong Type!"<<endl;
                return -1;
            }
    
    pos1 = pos2+1;
    //处理末尾空格
    while (pos1<=attDef.length()-1&&(attDef[pos1]==' '||attDef[pos1]=='\n')) pos1++;
    if (pos1>attDef.length()-1)
    {
        Att.attr_key_type = OTHER;
        return 1;
    }
    //如果后面还有有意义的字符串，就是约束
    pos2 = min(attDef.find(' ',pos1),attDef.find('\n',pos1));
    if (attDef.substr(pos1,pos2-pos1)=="unique")
    {
        Att.attr_key_type = UNIQUE;
        return 1;
    }
    else if (attDef.substr(pos1,pos2-pos1)=="primary")
    {
        Att.attr_key_type = PRIMARY;
        return 1;
    }
    else
    {
        cerr << "ERROR:" << "Wrong Constraint!"<<endl;
        Att.attr_key_type = -1;
        return -1;
    }
    
    return 1;
}

void API(condition &SQLCondition)
{
    
    int instruction;
    instruction = SQLCondition.instructionType;
    switch (instruction)
    {
        case INSERT:
        {
            string table_name = SQLCondition.tableName;
            API_Insert(table_name,SQLCondition.insertValues);
            break;
        }
        case SELECT:
        {
            vector<string> attrs = SQLCondition.colName;
            API_Select(SQLCondition.tableName ,attrs, SQLCondition.whereClause);
            break;
        }
        case DELETE:
        {
            API_Delete(SQLCondition.tableName, SQLCondition.whereClause)
            break;
        }
        case CREATE_TABLE:
        {
            Table tb = new Table;
            tb.table_name = SQLCondition.tableName;
            Attribute Attr = new Attribute;
            string att = SQLCondition.attribute;
            int pos1 = 0, pos2;
            int i = 0,flag = 0;
            string PKey = "";
            while(1)
            {
                pos2 = att.find(',',pos1);
                if(pos2 == -1)
                    break;   //to the end
                if(att.substr(pos1,7)!="primary")
                {
                    tb.attr_num ++;
                    Attr.attr_name = "";
                    Attr.attr_type = -1;
                    int res = attrDef(Attr,att.substr(pos1,pos2-pos1));
                    if(res != 1) return res;
                    tb.attrs.push_back(Attr);
                    if(Attr.attr_key_type == PRIMARY)
                        if(flag)
                        {
                            cerr<<"ERROR:"<<"more than one Primary key"<<endl;
                            return DEFINATION_ERROR;
                        }
                        else
                            flag = 1;
                    i++;
                    pos1 = pos2 +1;
                    while(att[pos1] == ' '||att[pos1] == '\n')
                        pos1 ++;
                }
                else
                {//set primary key sentence
                    if(flag)
                    {
                        cerr<<"ERROR:"<<"more than one Primary key"<<endl;
                        return DEFINATION_ERROR;
                    }
                    else
                        flag = 1;
                    pos1 = Attributes.find('(',pos1);
                    pos2 = Attributes.find(')',pos1+1);
                    PKey = att.substr(pos1+1,pos2-pos1-1);
                    pos1 = att.find(',',pos2)+1;
                    
                }
            }
            if(PKey != "" )
            {
                flag =0;
                for(i = 0; i <tb.attr_num; i ++)
                    if(tb.attrs[i].attr_name == PKey)
                    {
                        tb.attrs[i].attr_key_type = PRIMARY;
                        flag = 1;
                        break;
                    }
                if(flag == 0)
                {
                    cerr<<"ERROR:"<<"No such Attribute!"<<endl;
                    return DEFINATION_ERROR;
                }
            }
            API_CreateTable(tb);
            break;
        }
        case CREATE_INDEX: {
            Index idx(SQLCondition.indexName, SQLCondition.tableName, SQLCondition.attribute);
            API_CreateIndex(idx);
            break;
        }
        case DROP_TABLE:
        {
            API_DropTable(SQLCondition.tableName);
            break;
        }
        case DROP_INDEX:
        {
            Index Dindex(SQLCondition.indexName, SQLCondition.tableName."");
            API_DropIndex(Dindex);
            free(Dindex);
            break;
        }
        default:
            break;
    }
}

int interpreter(string &SQLSentence, condition &SQLCondition)
{
    string firstWord;
    int SQLCurrentPointer = 0, end = 0;
    int code = 0;
    
    //////////////////////input part/////////////////////////////////////////////
    if(SQLSentence.empty())
        SQLSentence = getInput();
    //////////////////////instruction analysis part//////////////////////////////////
    
    while (SQLSentence[SQLCurrentPointer] == ' ')//get rid of the ' ' from the beginning of the sentence
        SQLCurrentPointer++;
    end = SQLSentence.find(' ', SQLCurrentPointer);
    if (end == -1)
    {
        end = SQLSentence.find(';', SQLCurrentPointer);//example:"quit;", then there does not have a ' '.
    }
    firstWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);//get the first word from SQL sentence
    SQLCurrentPointer = end;//pointer move forward
    
    
    
    ///////start to analysis the sentence
    if (firstWord == "select")
    {
        code = select_clause(SQLSentence, SQLCurrentPointer, end, SQLCondition);
    }
    else if (firstWord == "insert")
    {
        code = insert_clause(SQLSentence, SQLCurrentPointer, end, SQLCondition);
    }
    else if (firstWord == "quit")
    {
        string nextWord;
        while (SQLSentence[SQLCurrentPointer] == ' ')//get rid of the ' ' after the first word
            SQLCurrentPointer++;
        end = SQLSentence.find(' ', SQLCurrentPointer);//get the next word
        nextWord = SQLSentence.substr(SQLCurrentPointer, end - SQLCurrentPointer);//check whether the quit instruction
        //has other words. example:"quit select;"
        if (nextWord == ";")//we can only find ';'
            return QUIT_NUMBER;
        else
            cerr << "ERROR:" << "Quit instruction should has only one key word \"quit\"\n";
    }
    else if (firstWord == "delete")
    {
        code = delect_clauese(SQLSentence, SQLCurrentPointer, end, SQLCondition);
    }
    else if (firstWord == "create")
    {
        code = create_clause(SQLSentence, SQLCurrentPointer, end, SQLCondition);
    }
/*    else if (firstWord == "exec")//execute the script
    {
        string temp;
        while (SQLSentence[SQLCurrentPointer] == ' ')//get rid of the ' ' from the beginning of the sentence
            SQLCurrentPointer++;
        end = SQLSentence.find(';', SQLCurrentPointer);
        --end;
        while (SQLSentence[end] != '>')
            --end;
        --end;
        while (SQLSentence[end] == ' ')
            --end;
        temp = SQLSentence.substr(SQLCurrentPointer + 1, end - SQLCurrentPointer);
        strcpy(address, temp.c_str());//copy the address to array
        execute_clause(temp, SQLCondition, BTree, bufManager);
        
        
        SQLCurrentPointer = end;
    }  //unfinished    */
    else if (firstWord == "drop")
    {
        code = drop_clause(SQLSentence, SQLCurrentPointer, end, SQLCondition);
    }
    else//input is wrong
    {
        cerr << "ERROR:" << "Doesn't found a instruction key word!" << endl;
        end = SQLSentence.find(';', 0);
        SQLSentence.erase(0, end + 1);
        return ERROR;
    }
    if (SQLSentence[SQLCurrentPointer + 1] == ';')
        SQLCurrentPointer += 2;
    else
    {
        while (SQLSentence[SQLCurrentPointer] == ' ')//get rid of the ' ' in the rest fo the sentence
            SQLCurrentPointer++;
        while (SQLSentence[SQLCurrentPointer] == ';')//find ';'
            SQLCurrentPointer++;
        
    }
    end = SQLSentence.find(';', 0);
    SQLSentence.erase(0, end + 1);//clear
    API(SQLCondition);
    return code;
}
