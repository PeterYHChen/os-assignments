#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>

using namespace std;

struct Process
{
    int a, b, c, m;
    int cpuBurst;
    int ioBurst;
};

int randomOS(int u)
{
    int rand = -1;
    ifstream randFile("random-numbers");
    if (!randFile.is_open())
    {
        cout << "Error. Fail to read random-numbers" << endl;
    }
    else
    {
        rand = 1;
    }
    randFile.close();

    return rand;
}

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        cout << "Error. Should run this program with a input file" << endl;
        return -1;
    }
    
    if (argc > 3)
    {
        cout << "Error. Too many arguments. Should only accept --verbose and a input file" << endl;
        return -1;
    }

    bool isVerbose = false;
    const char* inputFilePath;

    if (argc == 2)
    {
        inputFilePath = argv[1];
    }
    else
    {
        if (strcmp(argv[1], "--verbose") != 0)
        {
            cout << "Error. This program only accepts --verbose flag" << endl;
            return -1;
        }
        isVerbose = true;
        inputFilePath = argv[2];
    }
   
    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open())
    {
        cout << "Error. File " << inputFilePath << " does not exists." << endl;
        return -1;
    }

    int n;
    vector<Process> procList;
    inputFile >> n;
    Process proc;
    while (n--)
    {
        inputFile >> proc.a >> proc.b >> proc.c >> proc.m;
        procList.push_back(proc);
    }
    inputFile.close();

    for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
    {
        cout << "Process " << i - procList.begin() << ":" << endl;
        printf("    (A, B, C, M) = (%d, %d, %d, %d)\n", i->a, i->b, i->c, i->m);
    }

    return 0;
}
