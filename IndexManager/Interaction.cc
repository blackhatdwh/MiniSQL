/*
 * Interaction.cc
 * Copyright (C) 2017 weihaodong <weihaodong@weihaodong-pc>
 *
 * Distributed under terms of the MIT license.
 */

#include <iostream>
#include <vector>
#include "Interaction.h"
using namespace std;


vector<vector<string> > QueryTableContent(string table_name){
    static int col_num = 0;
    vector<vector<string> > result;
    vector<string> temp;
    temp.push_back("01");
    temp.push_back("a");
    temp.push_back("M");
    temp.push_back(to_string(col_num));
    result.push_back(temp);
    temp.clear();

    temp.push_back("02");
    temp.push_back("b");
    temp.push_back("M");
    temp.push_back("100");
    result.push_back(temp);
    temp.clear();

    temp.push_back("03");
    temp.push_back("c");
    temp.push_back("F");
    temp.push_back("200");
    result.push_back(temp);
    temp.clear();
    return result;
}

vector<string> QueryLatestRecord(){
    vector<string> temp;
    temp.push_back("04");
    temp.push_back("d");
    temp.push_back("M");
    temp.push_back("300");
    return temp;
}

vector<vector<string> > QueryDeletedContent(){
    vector<vector<string> > result;
    vector<string> temp;
    temp.push_back("01");
    temp.push_back("a");
    temp.push_back("M");
    result.push_back(temp);
    temp.clear();

    temp.push_back("02");
    temp.push_back("b");
    temp.push_back("M");
    result.push_back(temp);
    temp.clear();

    temp.push_back("03");
    temp.push_back("c");
    temp.push_back("F");
    result.push_back(temp);
    temp.clear();
    return result;
}
