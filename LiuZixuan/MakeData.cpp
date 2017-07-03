#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#define MAXBLOCKNUMBER 1000
#define BLOCKSIZE 4096
#define NOTFOUND -1
#define EMPTY '@'
#define END '#'

using namespace std;

int getId()
{
    return rand() % 1000;
}

int getGPA()
{
    return rand()%4 + rand() * 1.0 / RAND_MAX;
}

void getName(char name[])
{
    for (int i = 0; i < 10; i++) {
        name[i] = 'a'+rand()%26;
    }
    name[10] = '\0';
}

int main()
{
    ofstream fout("student.data");
    int id;
    char name[50] = {0};
    float GPA;

    int tupleLength = sizeof(int) + sizeof(float) + 50;

    srand(time(NULL));

    int num = 10;
    // for (int i = 0; i < 1; i++) {
        // fout.seekp(BLOCKSIZE*i, fout.beg);
        // for (int j = 0; j < BLOCKSIZE/tupleLength; j++) {
        for (int j = 0; j < num; j++) {
            id = getId();
            getName(name);
            GPA = getGPA();
            fout.write((char *)&id, sizeof(int));
            fout.write(name, sizeof(name));
            fout.write((char *)&GPA, sizeof(float));
            // cout << id << " " << name << " " << GPA << endl;
        }
        char ch = END;
        fout.write(&ch, 1);
    // }

    fout.close();
    cout << "== close ==" << endl;
}