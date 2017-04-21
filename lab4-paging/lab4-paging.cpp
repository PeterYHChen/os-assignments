#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <stdlib.h>

using namespace std;

#ifndef DEBUGMODE
#define DEBUGMODE true
#endif

#ifndef D
#define DEBUG(x) if (DEBUGMODE) {x}
#endif

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif
struct Process
{
    int id;
    vector<bool> pageAllocated;
    int size;
    int refCnt;
    double A, B, C;
    int refIndex;
};

struct Page
{
    int allocatedTime;
    int referencedTime;
    int startRefIndex;
    int processId; // processId = -1 indicates avaliable frame
};

vector<int> randNums;
vector<Page> frameTable; // size = M/S
vector<Process> procList;

int machineSize, pageSize, procSize, jobMix, refCnt;
string algorithm;

int getRandomNumber()
{
    static int pos = 0;

    if (pos == randNums.size())
        pos = 0;

    DEBUG(printf("use random number: %d\n", randNums[pos]);)
    return randNums[pos++];
}

void printVector(vector<Process> &procList)
{
    for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
    {
        printf("Process id: %d\n", i->id);
        printf("    page table size: %d\n", i->pageAllocated.size());
        printf("    size: %d\n", i->size);
        printf("    refCnt: %d\n", i->refCnt);
        printf("    refIndex: %d\n", i->refIndex);
        printf("    A: %1.3f B: %1.3f C: %1.3f\n", i->A, i->B, i->C);
    }
}

void initiate(int machineSize, int pageSize, int procSize, int jobMix, int refCnt)
{
    frameTable.clear();
    procList.clear();

    // initiate frame table
    frameTable.resize(machineSize / pageSize);
    for (vector<Page>::iterator iter = frameTable.begin(); iter != frameTable.end(); iter++)
        iter->processId = -1; // indicate empty frame

    // initiate vector of process list
    Process temp;
    temp.pageAllocated.resize(procSize/pageSize + 1, false);
    temp.size = procSize;
    temp.refCnt = refCnt;
    temp.refIndex = -1; // indicates no referenceIndex at the beginning

    switch (jobMix)
    {
        case 1:
            temp.id = 1;
            temp.A = 1;
            temp.B = 0;
            temp.C = 0;
            procList.push_back(temp);
            break;
        case 2:
            temp.A = 1;
            temp.B = 0;
            temp.C = 0;
            for (int i = 0; i < 4; i++)
            {
                temp.id = i+1;
                procList.push_back(temp);
            }
            break;
        case 3:
            temp.A = 0;
            temp.B = 0;
            temp.C = 0;
            for (int i = 0; i < 4; i++)
            {
                temp.id = i+1;
                procList.push_back(temp);
            }
            break;
        case 4:
            temp.id = 1;
            temp.A = 0.75;
            temp.B = 0.25;
            temp.C = 0;
            procList.push_back(temp);
            temp.id = 2;
            temp.A = 0.75;
            temp.B = 0;
            temp.C = 0.25;
            procList.push_back(temp);
            temp.id = 3;
            temp.A = 0.75;
            temp.B = 0.125;
            temp.C = 0.125;
            procList.push_back(temp);
            temp.id = 4;
            temp.A = 0.5;
            temp.B = 0.125;
            temp.C = 0.125;
            procList.push_back(temp);
            break;
        default:
            cout << "Error. Unrecognized jobMix value: " << jobMix << endl;
    }
}

void LIFO()
{

}

void RANDOM()
{
    
}

class compare
{
public:
    bool operator() (const Page* a, const Page* b) const
    {
        return a->referencedTime > b->referencedTime;
    }
};

void LRU()
{
    int time = 0;
    int quantum = 3;

    priority_queue<Page*, vector<Page*>, compare> lruQ;

    while (true)
    {
        int noRefProcessCnt = 0;
        for (vector<Process>::iterator currProc = procList.begin(); currProc != procList.end(); currProc++)
        {
            // no more reference of current process
            if (currProc->refCnt <= 0)
            {
                noRefProcessCnt++;
                continue;
            }

            // begin round robin of quantum 3
            for (int i = 0; i < quantum; i++)
            {
                time++;
                // the first reference, 111*k mod S
                if (currProc->refIndex < 0)
                    currProc->refIndex = (currProc->id * 111) % currProc->size;
                else
                {
                    double y = getRandomNumber() / (RAND_MAX + 1.0);
                    if (y < currProc->A)
                        currProc->refIndex += 1;
                    else if (y < currProc->A + currProc->B)
                        currProc->refIndex -= 5;
                    else if (y < currProc->A + currProc->B + currProc->C)
                        currProc->refIndex += 4;
                    else
                        currProc->refIndex = getRandomNumber();

                    currProc->refIndex %= currProc->size;
                }

                int refPageIndex = currProc->refIndex/pageSize;
                DEBUG(printf("references word %d (page %d) at time %d", currProc->refIndex, refPageIndex, time);)

                // if this reference index has been allocated to a frame
                if (currProc->pageAllocated[refPageIndex])
                {
                    DEBUG(printf(" - Hit in frame\n");)
                }
                // not allocate to a frame before
                else
                {
                    bool finishAllocation = false;

                    // search frame table from the end to the beginning for free frame
                    for (vector<Page>::iterator currPage = frameTable.end() - 1; currPage >= frameTable.begin(); currPage--)
                    {
                        if (finishAllocation)
                            break;

                        // find a free frame for a page
                        if (currPage->processId < 0)
                        {
                            currPage->allocatedTime = time;
                            currPage->referencedTime = time;
                            currPage->startRefIndex = currProc->refIndex;
                            currPage->processId = currProc->id;

                            // loop over words in the same page to indicate being allocated
                            int currRefIndex = currProc->refIndex + j;
                            for (int j = 0; j < pageSize; j++)
                            {
                                // ensure reference index is inside the process 
                                int currRefIndex = currProc->refIndex + j;
                                if (currRefIndex >= currProc->size)
                                    break;
                                currProc->pageAllocated[currProc->refIndex + j] = true;
                            }

                            finishAllocation = true;
                        }
                    }

                    // frame table is full, evict a page, record residence time
                    if (!finishAllocation)
                    {
                        // search for the least recently used (referenced) page to evict

                    }
                }
            }
        }

        // all references have been performed, finish lru algorithm
        if (noRefProcessCnt == procList.size())
            break;
    }

    // output summary
}

int main(int argc, char const *argv[])
{
    if (argc != 7)
    {
        cout << "Error. Should run this program with 6 arguments: ./a.out N P S J N R" << endl;
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
        int num;
        while(randFile >> num)
            randNums.push_back(num);
        randFile.close();
    }

    machineSize = atoi(argv[1]);
    pageSize = atoi(argv[2]);
    procSize = atoi(argv[3]);
    jobMix = atoi(argv[4]);
    refCnt = atoi(argv[5]);
    algorithm = argv[6];

    DEBUG(cout << machineSize << " " << pageSize << " " << procSize << " "  << jobMix << " "  << refCnt << " "  << algorithm << endl;)

    // initiate vector of process list and frame table
    initiate(machineSize, pageSize, procSize, jobMix, refCnt);
    DEBUG(printVector(procList);)

    if (algorithm == "lifo")
        LIFO();
    else if (algorithm == "random")
        RANDOM();
    else if (algorithm == "lru")
        LRU();
    else
    {
        cout << "Error. Replacement algorithm '" << algorithm << "' not exists" << endl;
        return -1;
    }

    return 0;
}
