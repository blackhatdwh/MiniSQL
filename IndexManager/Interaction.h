/*
 * Interaction.h
 * Copyright (C) 2017 weihaodong <weihaodong@weihaodong-pc>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef INTERACTION_H
#define INTERACTION_H
#include <vector>
using namespace std;

vector<vector<string> > QueryTableContent(string table_name);
vector<string> QueryLatestRecord();
vector<vector<string> > QueryDeletedContent();



#endif /* !INTERACTION_H */
