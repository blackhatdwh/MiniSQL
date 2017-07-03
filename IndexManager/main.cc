/*
 * main.cc
 * Copyright (C) 2017 weihaodong <weihaodong@weihaodong-pc>
 *
 * Distributed under terms of the MIT license.
 */

#include <iostream>
#include "IndexManager.h"
#include "Interaction.h"

using namespace std;

int main(){
    IndexManager idx;

    cout << "=============test create index and check exist===========\n";
    cout << "Now we have a table called student like this:\n";
    cout << "Offset is not in the real table. It represents the offset of the record in the file.\n";
    cout << "id\tname\tgender\toffset\n";
    cout << "01\ta\tM\t0\n";
    cout << "02\tb\tM\t100\n";
    cout << "03\tc\tF\t200\n";
    cout << "Now we have no index on the table. Check if there is a index on table student column 0\n";
    cout << idx.CheckExist("student", 0) << endl;
    cout << "Now create an index on table student, column 0, the column of ID.\n";
    idx.Create("student", 0, "fuck");
    cout << "Already created. Check if exist.\n";
    cout << idx.CheckExist("student", 0) << endl;

    cout << "\n============test search====================\n";
    cout << "Now the information in table student is already added into the index.\n";
    cout << "Test search.\n";
    cout << "Search for 01, 02, 03 on the index of table student, column 0.\n";
    cout << idx.Search("student", 0, "01") << endl;
    cout << idx.Search("student", 0, "02") << endl;
    cout << idx.Search("student", 0, "03") << endl;
    cout << "As you can see, the offsets are returned correspondingly.\n";

    cout << "\n==============test insert==================" << endl;
    cout << "Now we have inserted a record into the table student.\n";
    cout << "The record looks like this:\n04\td\t\t300\n";
    idx.Insert("student");
    cout << "Insertion complete. Now try to search for 04 on table student, column 0.\n";
    cout << idx.Search("student", 0, "04") << endl;


    cout << "\n=============test multi index on one table.=============\n";
    cout << "Now we have no index on table student, column 1. Check to prove:\n";
    cout << idx.CheckExist("student", 1) << endl;
    cout << "Create a index on table student, column 1, the column of name.\n";
    cout << "The name of the index is demo.\n";
    idx.Create("student", 1, "demo");
    cout << "Check if there is an index on table student, column 1.\n";
    cout << idx.CheckExist("student", 1) << endl;

    cout << "Search for a,b,c on the index on table student, column 1.\n";
    cout << idx.Search("student", 1, "a") << endl;
    cout << idx.Search("student", 1, "b") << endl;
    cout << idx.Search("student", 1, "c") << endl;
    cout << "As you can see, the corresponding offset are returned.\n";

    cout << "\n==============test delete record================\n";
    cout << "Delete the first record in the table student.\n";
    idx.DeleteKey("student");
    cout << "Search for a,b,c on the index on table student, column 1.\n";
    cout << idx.Search("student", 1, "a") << endl;
    cout << idx.Search("student", 1, "b") << endl;
    cout << idx.Search("student", 1, "c") << endl;
    cout << "As you can see, the first record can't be found.\n";
    cout << "Search for 01,02,03 on the index on table student, column 0.\n";
    cout << idx.Search("student", 0, "01") << endl;
    cout << idx.Search("student", 0, "02") << endl;
    cout << idx.Search("student", 0, "03") << endl;
    cout << "As you can see, the first record can't be found.\n";

    cout << "\n===============test delete index====================\n";
    cout << "Now we delete the index on talbe student, column 1.\n";
    idx.DeleteIndex("demo");
    cout << "Check if the index still exists.\n";
    cout << idx.CheckExist("student", 1) << endl;
    cout << "Search for a,b,c on the index on table student, column 1.\n";
    cout << idx.Search("student", 1, "a") << endl;
    cout << idx.Search("student", 1, "b") << endl;
    cout << idx.Search("student", 1, "c") << endl;
    cout << "As you can see, all records can't be found.\n";
}
