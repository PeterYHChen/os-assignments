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
    vector<int> pageToFrameIndex;
    int size;
    int refCnt;
    double A, B, C;
    int refIndex;
    int nextRefIndex;
    int pageFault;
    double totalResidencyTime;
    int numOfEvictions;
};

struct Page
{
    int allocatedTime;
    int referencedTime;
    int refPageIndex;
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

    // DEBUG(printf("use random number: %d\n", randNums[pos]);)
    return randNums[pos++];
}

void printVector(vector<Process> &procList)
{
    for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
    {
        printf("Process id: %d\n", i->id);
        printf("    page table size: %ld\n", i->pageToFrameIndex.size());
        printf("    size: %d\n", i->size);
        printf("    refCnt: %d\n", i->refCnt);
        printf("    refIndex: %d\n", i->refIndex);
        printf("    A: %1.3f B: %1.3f C: %1.3f\n", i->A, i->B, i->C);
    }
    cout << endl;
}

void initiate(int machineSize, int pageSize, int procSize, int jobMix, int refCnt)
{
    frameTable.clear();
    procList.clear();

    // initiate frame table
    frameTable.resize(machineSize / pageSize);
    for (vector<Page>::iterator iter = frameTable.begin(); iter != frameTable.end(); iter++)
        iter->processId = -1; // indicate empty frame

    // initiate vector of process list, init page to frame index to be -1 indicates not allocated
    Process proc;
    proc.pageToFrameIndex.resize(procSize/pageSize + 1, -1);
    proc.size = procSize;
    proc.refCnt = refCnt;
    proc.refIndex = -1; // indicates no referenceIndex at the beginning
    proc.nextRefIndex -1;
    proc.pageFault = 0;
    proc.totalResidencyTime = 0;
    proc.numOfEvictions = 0;

    switch (jobMix)
    {
        case 1:
            proc.id = 1;
            proc.A = 1;
            proc.B = 0;
            proc.C = 0;
            procList.push_back(proc);
            break;
        case 2:
            proc.A = 1;
            proc.B = 0;
            proc.C = 0;
            for (int i = 0; i < 4; i++)
            {
                proc.id = i+1;
                procList.push_back(proc);
            }
            break;
        case 3:
            proc.A = 0;
            proc.B = 0;
            proc.C = 0;
            for (int i = 0; i < 4; i++)
            {
                proc.id = i+1;
                procList.push_back(proc);
            }
            break;
        case 4:
            proc.id = 1;
            proc.A = 0.75;
            proc.B = 0.25;
            proc.C = 0;
            procList.push_back(proc);
            proc.id = 2;
            proc.A = 0.75;
            proc.B = 0;
            proc.C = 0.25;
            procList.push_back(proc);
            proc.id = 3;
            proc.A = 0.75;
            proc.B = 0.125;
            proc.C = 0.125;
            procList.push_back(proc);
            proc.id = 4;
            proc.A = 0.5;
            proc.B = 0.125;
            proc.C = 0.125;
            procList.push_back(proc);
            break;
        default:
            cout << "Error. Unrecognized jobMix value: " << jobMix << endl;
    }
}

void printSummary()
{
    cout << endl;

    int totalPageFault = 0;
    double totalResidencyTime = 0;
    int totalNumOfEvictions = 0;
    for (vector<Process>::iterator currProc = procList.begin(); currProc != procList.end(); currProc++)
    {
        totalPageFault += currProc->pageFault;
        totalResidencyTime += currProc->totalResidencyTime;
        totalNumOfEvictions += currProc->numOfEvictions;
        if (currProc->numOfEvictions > 0)
            printf("Process %d had %d faults and %f average residency.\n", 
                currProc->id, currProc->pageFault, currProc->totalResidencyTime / currProc->numOfEvictions);
        else
            printf("Process %d had %d faults.\n\tWith no evictions, the average residence is undefined.\n", 
                currProc->id, currProc->pageFault);

    }

    if (totalNumOfEvictions > 0)
        printf("\nThe total number of faults is %d and the overall average residency is %f.\n\n", 
            totalPageFault, totalResidencyTime / totalNumOfEvictions);
    else
        printf("\nThe total number of faults is %d.\n\tWith no evictions, the overall average residence is undefined.\n\n", 
            totalPageFault);
}
void LIFO()
{

}

void RANDOM()
{
    
}

void LRU()
{
    int time = 0;
    int quantum = 3;

    while (true)
    {
        int noRefProcessCnt = 0;
        for (vector<Process>::iterator currProc = procList.begin(); currProc != procList.end(); currProc++)
        {
            // begin round robin of quantum 3
            for (int i = 0; i < quantum; i++)
            {
                // no more reference of current process
                if (currProc->refCnt <= 0)
                {
                    noRefProcessCnt++;
                    break;
                }
                currProc->refCnt--;

                time++;

                // if this is the first reference, 111*k mod S
                if (currProc->refIndex < 0)
                    currProc->refIndex = (currProc->id * 111) % currProc->size;
                else
                    currProc->refIndex = currProc->nextRefIndex;

                int currRefPageIndex = currProc->refIndex/pageSize;
                DEBUG(printf("Process %d references word %d (page %d) at time %d", currProc->id, currProc->refIndex, currRefPageIndex, time);)

                // if this reference index has been allocated to a frame
                int currFrameIndex = currProc->pageToFrameIndex[currRefPageIndex];
                if (currFrameIndex >= 0)
                {
                    // update referenced time of the page in frame
                    frameTable[currFrameIndex].referencedTime = time;
                    DEBUG(printf(" - Hit in frame %d\n", currFrameIndex);)
                }
                // not allocate to a frame before, page fault
                else
                {
                    currProc->pageFault++;

                    int targetFrameIndex;
                    vector<Page>::iterator targetPage;
                    // search frame table from the end to the beginning for free frame
                    for (targetPage = frameTable.end() - 1; targetPage >= frameTable.begin(); targetPage--)
                    {
                        targetFrameIndex = targetPage - frameTable.begin();

                        // id < 0 means found a free frame for a page
                        if (targetPage->processId < 0)
                        {
                            DEBUG(printf(" - Fault, using free frame %d\n", targetFrameIndex);)
                            break;
                        }
                    }

                    // if frame table is full, evict a page, record residence time
                    if (targetPage < frameTable.begin() || targetPage >= frameTable.end())
                    {
                        // search for the least recently used (referenced) page as the target page
                        targetPage = frameTable.begin();
                        targetFrameIndex = targetPage - frameTable.begin();
                        int lruTime = targetPage->referencedTime;

                        for (vector<Page>::iterator currPage = frameTable.begin() + 1; currPage != frameTable.end(); currPage++)
                        {
                            if (lruTime > currPage->referencedTime)
                            {
                                lruTime = currPage->referencedTime;
                                targetPage = currPage;
                                targetFrameIndex = targetPage - frameTable.begin();
                            }
                        }

                        DEBUG(printf(" - Fault, evicting page %d of process %d from frame %d\n", targetPage->refPageIndex, targetPage->processId, targetFrameIndex);)

                        // calculate residence time
                        procList[targetPage->processId - 1].numOfEvictions++;
                        procList[targetPage->processId - 1].totalResidencyTime += time - targetPage->allocatedTime;

                        // evict the least recently used (referenced) page
                        procList[targetPage->processId - 1].pageToFrameIndex[targetPage->refPageIndex] = -1;
                        targetPage->processId = -1;

                    }

                    // allocate page to current frame
                    targetPage->allocatedTime = time;
                    targetPage->referencedTime = time;
                    targetPage->refPageIndex = currRefPageIndex;
                    targetPage->processId = currProc->id;

                    // log the page to frame index
                    currProc->pageToFrameIndex[currRefPageIndex] = targetFrameIndex;

                }

                // calculate the next reference index
                double y = getRandomNumber() / (RAND_MAX + 1.0);
                currProc->nextRefIndex = currProc->refIndex;
                if (y < currProc->A)
                    currProc->nextRefIndex += 1;
                else if (y < currProc->A + currProc->B)
                    currProc->nextRefIndex += currProc->size - 5;
                else if (y < currProc->A + currProc->B + currProc->C)
                    currProc->nextRefIndex += 4;
                else
                    currProc->nextRefIndex = getRandomNumber();

                currProc->nextRefIndex %= currProc->size;
            }
        }

        // all references have been performed, finish lru algorithm
        if (noRefProcessCnt == procList.size())
            break;
    }

    // output summary
    printSummary();
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

    cout << "The machine size is " << machineSize << endl;
    cout << "The page size is " << pageSize << endl;
    cout << "The process size is " << procSize << endl;
    cout << "The job mix number is " << jobMix << endl;
    cout << "The number of references per process is " << refCnt << endl;
    cout << "The replacement algorithm is " << algorithm << endl << endl;

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
