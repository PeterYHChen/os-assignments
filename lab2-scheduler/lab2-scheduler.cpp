#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <deque>

using namespace std;

enum State
{
    UNSTARTED, READY, RUNNING, BLOCKED, TERMINATED
};

struct Process
{
    int a, b, c, m;

    State state;
    
    int arrivalTime;
    int remainCPU;
    int remainCPUBurst;
    int remainIOBurst;

    int finishTime;
    int turnaroundTime;
    int ioTime;
    int waitingTime;
};

vector<int> randNums;
bool isVerbose = false;

int randomOS(int u)
{
    static int pos = 0;
    // reset pos to the first random numeber in file
    if (u <= 0)
    {
        pos = 0;
        return -1;
    }

    if (pos == randNums.size())
        pos = 0;

    // printf("randNums %d %% %d + 1 = %d\n", randNums[pos], u, randNums[pos] % u + 1);
    return randNums[pos++] % u + 1;
}

void printVector(vector<Process> &vec)
{
    cout << vec.size();
    for (vector<Process>::iterator i = vec.begin(); i != vec.end(); ++i)
        printf(" (%d, %d, %d, %d)", i->a, i->b, i->c, i->m);
}

void printSummary(vector<Process> &vec, double ioUti)
{
    int finishTime = 0;
    double cpuUti = 0.0;
    double throughput = 0.0;
    double averageTurn = 0.0;
    double averageWait = 0.0;

    for (vector<Process>::iterator i = vec.begin(); i != vec.end(); ++i)
    {
        finishTime = finishTime < i->finishTime? i->finishTime:finishTime;
        cpuUti += i->c;
        averageTurn += i->turnaroundTime;
        averageWait += i->waitingTime;

        cout << "Process " << i - vec.begin() << ":" << endl;
        printf("         (A,B,C,M) = (%d,%d,%d,%d)\n", i->a, i->b, i->c, i->m);
        printf("         Finishing Time: %d\n", i->finishTime);
        printf("         Turnaround Time: %d\n", i->turnaroundTime);
        printf("         I/O Time: %d\n", i->ioTime);
        printf("         Waiting Time: %d\n", i->waitingTime);
        cout << endl;
    }

    cpuUti /= finishTime;
    throughput = vec.size() / (double)finishTime * 100.0;
    averageTurn /= vec.size();
    averageWait /= vec.size();

    cout << "Summary Data: " << endl;
    printf("         Finishing Time: %d\n", finishTime);
    printf("         CPU Utilization: %f\n", cpuUti);
    printf("         I/O Utilization: %f\n", ioUti);
    printf("         Throughput: %f processes per hundred cycles\n", throughput);
    printf("         Average turnaround time: %f\n", averageTurn);
    printf("         Average waiting time: %f\n", averageWait);
    cout << endl;
}

bool sortByArrival(Process a, Process b)
{
    return a.a < b.a;
}

void FCFS(vector<Process> procList)
{
    cout << endl << "The original input was: ";
    printVector(procList);
    cout << endl;
    sort(procList.begin(), procList.end(), sortByArrival);
    cout << "The (sorted) input is:  ";
    printVector(procList);
    cout << endl << endl;

    // initialization
    queue<int> readyQ;

    // start timing
    int timer = 0;
    int terminatedCnt = 0;
    double blockCycleCnt = 0;
    while (terminatedCnt < procList.size())
    {
        bool isRunning = false;

        if (isVerbose)
            printf("Before cycle %3d:", timer);

        // state changed after previous cycle
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    if (isVerbose)
                        printf("%15s %4d", "unstarted", 0);

                    if (i->arrivalTime == 0)
                    {
                        i->state = READY;
                        readyQ.push(i - procList.begin());
                    }
                    break;

                case READY:
                    if (isVerbose)
                        printf("%15s %4d", "ready", 0);
                    break;
                
                case RUNNING:
                    if (isVerbose)
                        printf("%15s %4d", "running", i->remainCPUBurst + 1);

                    if (i->remainCPU == 0)
                    {
                        terminatedCnt++;
                        i->state = TERMINATED;
                        i->finishTime = timer;
                        i->turnaroundTime = i->finishTime - i->a;
                    }
                    else if (i->remainCPUBurst == 0)
                    {
                        i->state = BLOCKED;
                    }
                    else
                        isRunning = true;
                    break;
                
                case BLOCKED:
                    if (isVerbose)
                        printf("%15s %4d", "blocked", i->remainIOBurst + 1);

                    if (i->remainIOBurst == 0)
                    {
                        i->state = READY;
                        readyQ.push(i - procList.begin());
                    }
                    break;
                
                case TERMINATED:
                    if (isVerbose)
                        printf("%15s %4d", "terminated", 0);
                    break;
            }
        }

        if (isVerbose)
            printf("\n");

        // if no running process and there are processes in the ready queue
        if (!isRunning && !readyQ.empty())
        {
            // find the earliest process in the ready queue
            int index = readyQ.front();
            readyQ.pop();
            vector<Process>::iterator temp = procList.begin() + index;
     
            temp->state = RUNNING;
            int rand = randomOS(temp->b);
            temp->remainCPUBurst = (rand > temp->remainCPU? temp->remainCPU:rand);
            temp->remainIOBurst = temp->remainCPUBurst * temp->m;
            // printf("remainCPUBurst: %d, remainIOBurst %d\n", temp->remainCPUBurst, temp->remainIOBurst);
        }

        // increase time count after current cycle
        bool isBlocked = false;
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    i->arrivalTime--;
                    break;
                case READY:
                    i->waitingTime++;
                    break;
                case RUNNING:
                    i->remainCPU--;
                    i->remainCPUBurst--;
                    break;
                case BLOCKED:
                    i->remainIOBurst--;
                    i->ioTime++;
                    isBlocked = true;
                case TERMINATED:
                    break;
            }
        }
        if (isBlocked)
            blockCycleCnt++;

        timer++;
    }

    cout << endl << "The scheduling algorithm used was First Come First Serve" << endl << endl;
    printSummary(procList, blockCycleCnt/(timer-1));
}

void RR(vector<Process> procList)
{

    cout << endl << "The original input was: ";
    printVector(procList);
    cout << endl;
    sort(procList.begin(), procList.end(), sortByArrival);
    cout << "The (sorted) input is:  ";
    printVector(procList);
    cout << endl << endl;

    // initialization
    queue<int> readyQ;

    // start timing
    int timer = 0;
    int terminatedCnt = 0;
    double blockCycleCnt = 0;
    int quantum = 2;
    while (terminatedCnt < procList.size())
    {
        bool isRunning = false;

        if (isVerbose)
            printf("Before cycle %3d:", timer);

        // state changed after previous cycle
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    if (isVerbose)
                        printf("%15s %4d", "unstarted", 0);

                    if (i->arrivalTime == 0)
                    {
                        i->state = READY;
                        readyQ.push(i - procList.begin());
                    }
                    break;

                case READY:
                    if (isVerbose)
                        printf("%15s %4d", "ready", 0);
                    break;
                
                case RUNNING:
                    if (isVerbose)
                        printf("%15s %4d", "running", quantum+1);

                    if (i->remainCPU == 0)
                    {
                        terminatedCnt++;
                        i->state = TERMINATED;
                        i->finishTime = timer;
                        i->turnaroundTime = i->finishTime - i->a;
                    }
                    else if (i->remainCPUBurst == 0)
                        i->state = BLOCKED;
                    else if (quantum <= 0)
                    {
                        i->state = READY;
                        readyQ.push(i - procList.begin());
                    }
                    else
                        isRunning = true;
                    break;
                
                case BLOCKED:
                    if (isVerbose)
                        printf("%15s %4d", "blocked", i->remainIOBurst + 1);

                    if (i->remainIOBurst == 0)
                    {
                        i->state = READY;
                        readyQ.push(i - procList.begin());
                    }
                    break;
                
                case TERMINATED:
                    if (isVerbose)
                        printf("%15s %4d", "terminated", 0);
                    break;
            }
        }

        if (isVerbose)
            printf("\n");

        // if no running process and there are processes in the ready queue
        if (!isRunning && !readyQ.empty())
        {
            // find the earliest process in the ready queue
            int index = readyQ.front();
            readyQ.pop();
            vector<Process>::iterator temp = procList.begin() + index;
     
            temp->state = RUNNING;
            if (temp->remainCPUBurst == 0)
            {
                int rand = randomOS(temp->b);
                temp->remainCPUBurst = (rand > temp->remainCPU? temp->remainCPU:rand);
                temp->remainIOBurst = temp->remainCPUBurst * temp->m;
            }
            quantum = 2;
            quantum = (quantum > temp->remainCPUBurst? temp->remainCPUBurst:quantum);
            // printf("remainCPUBurst: %d, remainIOBurst %d\n", temp->remainCPUBurst, temp->remainIOBurst);
        }

        // increase time count after current cycle
        bool isBlocked = false;
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    i->arrivalTime--;
                    break;
                case READY:
                    i->waitingTime++;
                    break;
                case RUNNING:
                    i->remainCPU--;
                    i->remainCPUBurst--;
                    quantum--;
                    break;
                case BLOCKED:
                    i->remainIOBurst--;
                    i->ioTime++;
                    isBlocked = true;
                case TERMINATED:
                    break;
            }
        }
        if (isBlocked)
            blockCycleCnt++;

        timer++;
    }

    cout << endl << "The scheduling algorithm used was Round Robbin with quantum 2" << endl << endl;
    printSummary(procList, blockCycleCnt/(timer-1));
}

void Uniprogrammed(vector<Process> procList)
{
    cout << endl << "The original input was: ";
    printVector(procList);
    cout << endl;
    sort(procList.begin(), procList.end(), sortByArrival);
    cout << "The (sorted) input is:  ";
    printVector(procList);
    cout << endl << endl;

    // initialization
    deque<int> readyQ;

    // start timing
    int timer = 0;
    int terminatedCnt = 0;
    double blockCycleCnt = 0;
    while (terminatedCnt < procList.size())
    {
        bool isRunning = false;

        if (isVerbose)
            printf("Before cycle %3d:", timer);

        // state changed after previous cycle
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    if (isVerbose)
                        printf("%15s %4d", "unstarted", 0);

                    if (i->arrivalTime == 0)
                    {
                        i->state = READY;
                        readyQ.push_back(i - procList.begin());
                    }
                    break;

                case READY:
                    if (isVerbose)
                        printf("%15s %4d", "ready", 0);
                    break;
                
                case RUNNING:
                    if (isVerbose)
                        printf("%15s %4d", "running", i->remainCPUBurst + 1);

                    if (i->remainCPU == 0)
                    {
                        terminatedCnt++;
                        i->state = TERMINATED;
                        i->finishTime = timer;
                        i->turnaroundTime = i->finishTime - i->a;
                    }
                    else if (i->remainCPUBurst == 0)
                    {
                        i->state = BLOCKED;
                        readyQ.push_front(i - procList.begin());
                    }
                    else
                        isRunning = true;
                    break;
                
                case BLOCKED:
                    if (isVerbose)
                        printf("%15s %4d", "blocked", i->remainIOBurst + 1);

                    if (i->remainIOBurst == 0)
                    {
                        i->state = READY;
                    }
                    break;
                
                case TERMINATED:
                    if (isVerbose)
                        printf("%15s %4d", "terminated", 0);
                    break;
            }
        }

        if (isVerbose)
            printf("\n");

        // if no running process and there are processes in the ready queue
        if (!isRunning && !readyQ.empty())
        {
            // find the earliest process in the ready queue
            int index = readyQ.front();
            vector<Process>::iterator temp = procList.begin() + index;
     
            if (temp->state == READY)
            {
                readyQ.pop_front();
                temp->state = RUNNING;
                int rand = randomOS(temp->b);
                temp->remainCPUBurst = (rand > temp->remainCPU? temp->remainCPU:rand);
                temp->remainIOBurst = temp->remainCPUBurst * temp->m;
            }
            // printf("remainCPUBurst: %d, remainIOBurst %d\n", temp->remainCPUBurst, temp->remainIOBurst);
        }

        // increase time count after current cycle
        bool isBlocked = false;
        for (vector<Process>::iterator i = procList.begin(); i != procList.end(); ++i)
        {
            switch (i->state)
            {
                case UNSTARTED:
                    i->arrivalTime--;
                    break;
                case READY:
                    i->waitingTime++;
                    break;
                case RUNNING:
                    i->remainCPU--;
                    i->remainCPUBurst--;
                    break;
                case BLOCKED:
                    i->remainIOBurst--;
                    i->ioTime++;
                    isBlocked = true;
                case TERMINATED:
                    break;
            }
        }
        if (isBlocked)
            blockCycleCnt++;

        timer++;
    }

    cout << endl << "The scheduling algorithm used was Uniprocessing" << endl << endl;
    printSummary(procList, blockCycleCnt/(timer-1));
}

void SJF(vector<Process> procList)
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
        int num;
        while(randFile >> num)
            randNums.push_back(num);
        randFile.close();
    }

    int n;
    vector<Process> procList;
    inputFile >> n;
    Process proc;
    while (n--)
    {
        char ch;
        inputFile >> ch >> proc.a >> proc.b >> proc.c >> proc.m >> ch;
        proc.state = UNSTARTED;
    
        proc.arrivalTime = proc.a;
        proc.remainCPU = proc.c;
        proc.remainCPUBurst = 0;
        proc.remainIOBurst = 0;

        proc.finishTime = 0;
        proc.turnaroundTime = 0;
        proc.ioTime = 0;
        proc.waitingTime = 0;

        procList.push_back(proc);
    }
    inputFile.close();

    while(true)
    {
        cout << "Please select a scheduling algorithm for simulation:" << endl;
        cout << "1. FCFS    2. RR with quantum 2    3. Uniprogrammed    4. SJF" << endl;
        char choice;
        cin >> choice;
        switch (choice - '0')
        {
            case 1:
                randomOS(-1);
                FCFS(procList);
                break;

            case 2:
                randomOS(-1);
                RR(procList);
                break;

            case 3:
                randomOS(-1);
                Uniprogrammed(procList);
                break;

            case 4:
                randomOS(-1);
                SJF(procList);
                break;

            default:
                cout << "No such choice, please select from 1 to 4" << endl;
        }
    }

    return 0;
}
