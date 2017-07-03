#include <iostream>
#include "IndexManager.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
using namespace std;

IndexManager::IndexManager(){
    tree_ = nullptr;
    directory_ = "";
    ofstream index_dic;
    index_dic.open("./.index_dic");
    index_dic.close();
}

IndexManager::~IndexManager(){
    delete tree_;
}


string IndexManager::IndexNameToIndex(string index_name){
    ifstream file;
    string index_name_in_dic;
    string index_directory;
    file.open("./.index_dic");
    while(!file.eof()){
        getline(file, index_name_in_dic);
        getline(file, index_directory);
        if(index_name_in_dic == index_name){
            file.close();
            return index_directory;
        }
    }
    file.close();
    return "";
}


bool IndexManager::CheckExist(string table_name, int col_num){
    string directory = GenerateIndexDirectory(table_name, col_num);
    ifstream check_stream(directory);
    if(check_stream.good()){
        return true;
    }
    else{
        return false;
    }
}
bool IndexManager::CheckExist(string directory){
    ifstream check_stream(directory);
    if(check_stream.good()){
        return true;
    }
    else{
        return false;
    }
}

void IndexManager::Create(string table_name, int col_num, string index_name){
    string directory = GenerateIndexDirectory(table_name, col_num);
    if(!CheckExist(directory)){

        // store the index_name <-> index file
        ofstream file;
        file.open("./.index_dic", ios::app);
        file << index_name << endl;
        file << directory << endl;
        file.close();

        // delete the previous tree
        delete tree_;
        // update the current index file directory
        directory_ = directory;
        // create a new b plus tree from empty
        tree_ = new BPlusTree(directory, false);
        // TODO
        // QueryTableContent(string table_name) will return the table content of table_name in vector of vector of string form.
        vector<vector<string> > table_content = QueryTableContent(table_name);
        for(int i = 0; i < table_content.size(); i++){
            //char* key_char = table_content[i][col_num].c_str();
            char* key_char = strdup(table_content[i][col_num].c_str());
            m_key_t temp_key(key_char);
            value_t temp_value = stoi(table_content[i].back());
            tree_->Insert(temp_key, temp_value);
        }
    }
}

void IndexManager::Insert(string table_name){
    vector<string> index_list = IndexColumnOfTable(table_name);
    // TODO
    // QueryLatestRecord() will return the content and offset of the lastest inserted record
    vector<string> latest_record_content = QueryLatestRecord();
    for(int i = 0; i < index_list.size(); i++){
        string directory = "./.index/" + table_name + "/" + index_list[i];
        if(directory != directory_){
            LoadIndex(directory);
        }
        char* key_char = strdup(latest_record_content[stoi(index_list[i])].c_str());
        m_key_t temp_key(key_char);
        value_t temp_value = stoi(latest_record_content.back());
        tree_->Insert(temp_key, temp_value);
    }
}

int IndexManager::Search(string table_name, int col_num, char* key){
    string directory = GenerateIndexDirectory(table_name, col_num);
    if(!CheckExist(directory)){
        return -1;
    }
    if(directory != directory_){
        LoadIndex(directory);
    }
    m_key_t temp_key(key);
    int result = tree_->Search(temp_key);
    return result;
}

void IndexManager::DeleteIndex(string index_name){
    string directory = IndexNameToIndex(index_name);
    remove(directory.c_str());
    if(directory == directory_){
        delete tree_;
        tree_ = nullptr;
    }
}

void IndexManager::DeleteIndex(string table_name, int col_num){
    string directory = GenerateIndexDirectory(table_name, col_num);
    remove(directory.c_str());
    if(directory == directory_){
        delete tree_;
        tree_ = nullptr;
    }
}

void IndexManager::DeleteKey(string table_name){
    vector<string> index_list = IndexColumnOfTable(table_name);
    // TODO
    vector<vector<string> > deleted_content = QueryDeletedContent();
    for(int i = 0; i < index_list.size(); i++){
        string directory = "./.index/" + table_name + "/" + index_list[i];
        if(directory != directory_){
            LoadIndex(directory);
        }
        //char* key_char = deleted_content[stoi((index_list[i].c_str()))];
        for(int j = 0; j < deleted_content.size(); j++){
            char* key_char = strdup((deleted_content[j][stoi(index_list[i])]).c_str());
            m_key_t temp_key(key_char);
            tree_->Delete(temp_key);
        }
    }
}


void IndexManager::LoadIndex(string directory){
    delete tree_;
    tree_ = new BPlusTree(directory, true);
    directory_ = directory;
}

string IndexManager::GenerateIndexDirectory(string table_name, int col_num){
    string prefix = "./.index/" + table_name + "/";
    struct stat tmp;
    if(stat(prefix.c_str(), &tmp) != 0){
        string command = "mkdir -p " + prefix;
        system(command.c_str());
    }
    return  prefix + to_string(col_num);
}

vector<string> IndexManager::IndexColumnOfTable(string table_name){
	vector<string> result;
	string directory = "./.index/" + table_name + "/";
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(directory.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
            string temp(ent->d_name);
            if(temp != "." && temp != ".."){
                result.push_back(temp);
            }
		}
		closedir (dir);
	}
    return result;
}
