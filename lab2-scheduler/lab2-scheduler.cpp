#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>

using namespace std;

enum State
{
    READY, BLOCKED, RUNNING
};

struct Process
{
    int a, b, c, m;
    int remainCPU;
    int remainBurst;
    State state;
};

int randNums[100001];

int randomOS(int u, bool reset)
{
    static int pos = 0;
    if (reset)
    {
        pos = 0;
    }
    return randNums[pos++] % u + 1;
}

void FCFS(vector<Process> procList, bool isVerbose)
{
    

}

void RR(vector<Process> procList, bool isVerbose)
{

}

void Uniprogrammed(vector<Process> procList, bool isVerbose)
{

}

void SJF(vector<Process> procList, bool isVerbose)
{

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

    ifstream randFile("random-numbers");
    if (!randFile.is_open())
    {
        cout << "Error. Fail to read file 'random-numbers'. Check current directory to ensure this file exists" << endl;
        return -1;
    }
    else
    {
        for (int i = 0; i < 100000; i++)
            randFile >> randNums[i];
        randFile.close();
    }

    int n;
    vector<Process> procList;
    inputFile >> n;
    Process proc;
    while (n--)
    {
        inputFile >> proc.a >> proc.b >> proc.c >> proc.m;
        proc.remainCPU = proc.c;
        proc.state = READY;
        procList.push_back(proc);
    }
    inputFile.close();

    for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
    {
        cout << "Process " << i - procList.begin() << ":" << endl;
        printf("    (A, B, C, M) = (%d, %d, %d, %d)\n", i->a, i->b, i->c, i->m);
    }

    FCFS(procList, isVerbose);
    RR(procList, isVerbose);
    Uniprogrammed(procList, isVerbose);
    SJF(procList, isVerbose);

    return 0;
}
