//
//  Interpreter.hpp
//  Minisql
//
//  Created by zhe on 2018/6/21.
//  Copyright © 2018年 zhezhezhe. All rights reserved.
//

#ifndef Interpreter_hpp
#define Interpreter_hpp
#include "Minisql.h"
#include <stdio.h>
#include "API.h"
#define PROMPT "\nminisql> "
/*#ifndef TOKENOP
#define TOKENOP
enum Tokenop
{
    AND = 258,
    ON = 259,
    SELECT = 260,
    FROM = 261,
    WHERE = 262,
    DROP = 263,
    TABLE = 264,
    CREATE = 265,
    INDEX = 266,
    PRIMARY = 267,
    KEY = 268,
    VALUES = 269,
    UNIQUE = 270,
    INSERT = 271,
    INTO = 272,
    DELETE = 273,
    QUIT = 274,
    EXECFILE = 275,
    INT = 276,
    FLOAT = 277,
    CHAR = 278,
    NAME = 279,
    STRING = 280,
    INTNUM = 281,
    FLOATNUM = 282,
    EMPTY = 283,
    COMPARISON = 284,
    STMT = 285,
    ATTR_INFO = 286,
    ATTR_LIST = 287,
    ATTR = 288,
    DATA_TYPE = 289,
    ATTR_VALUE_LIST = 290,
    WH_LIST = 291,
    WH_NAME = 292
};
#endif*/
#define ERROR -10001
#define INSERT 258
#define SELECT 259
#define DELETE 260
#define CREATE_TABLE 261
#define CREATE_INDEX 262
#define DROP_TABLE 263
#define DROP_INDEX 264

using namespace std;
class condition
{
public:
    int instructionType;
    vector <string> colName;
    string tableName;
    string indexName;
    string whereClause;
    string insertValues;
    string attribute;
public:
    void setInstruction(int Ins);
    void setColName(string ColName)
    {
        if(ColName == "*");
        
    }
    void setInstruction(int type) { instructionType = type; }//set the instruction type
	void setColName(const string &col) { colName.push_back(col); }
	void setTableName(const string &table) { tableName = table; }
	void setIndexName(const string &index) { indexName = index; }
	void setWhereClause(const string &whereC) { whereClause =whereC; }
	void setInsertValues(const string &insertV) { insertValues = insertV; }
	void setAttribute(const string &attributeS) { attribute = attributeS; }
};

string getInput();
int attrDef(Attribute Att ,string attDef)
int select_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition);
int drop_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition);
int insert_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition);
int create_clause(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition);
int delect_clauese(string &SQLSentence, int &SQLCurrentPointer, int &end, condition &SQLCondition);
int interpreter(string &SQLSentence, condition &SQLCondition, BPLUSTREE &BTree, buffermanager &bufManager);
void API(condition &SQLCondition, BPLUSTREE &BTree, buffermanager &bufManager);
class Interpreter {
public:
    Interpreter()
    {
        //welcome user interface
        cout << "Welcome to the minisql system."<< endl;
        cout << PROMPT;
    }
    
    ~Interpreter()
    {
        cout << endl << "Good Bye. " << endl;
    }
    
    void exec()
    {
        Analysis();
    }
    
};
#endif /* Interpreter_hpp */
