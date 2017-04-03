#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <map>

using namespace std;

enum Activity
{
    INITIATE, REQUEST, RELEASE, COMPUTE, TERMINATE
};

struct Command 
{
    string a;
    int b, c, d;
};

struct Task
{
    int taskNumber;
    vector<int> claimList;
    vector<int> resrcList;
    int takenTime;
    int waitingTime;
};

map<string, Activity> s_mapActivity;

// generates input data for testing
void testInput(
    vector<Task> taskList, 
    vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList);

void output(vector<Task> taskList);

// map each activity to an enum value
int initMap()
{
    // map each activity to an enum value
    s_mapActivity["initiate"] = INITIATE;
    s_mapActivity["request"] = REQUEST;
    s_mapActivity["release"] = RELEASE;
    s_mapActivity["compute"] = COMPUTE;
    s_mapActivity["terminate"] = TERMINATE;
}

// class compare
// {
// public:
//     bool operator() (const Process* a, const Process* b) const
//     {
//         return a->remainCPU > b->remainCPU;
//     }
// };

vector<Task> optimisticResourceManager(
    vector<Task> taskList, 
    vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList)
{
    int cycle = 0;
    int taskNum = taskList.size();

    int terminateCnt = 0;

    while (true)
    {
        int waitingCnt = 0;
        Command cmd;
        vector<int> releaseResourceList(totalResourceList.size(), 0);

        if (terminateCnt >= taskNum)
            break;

        for (int i = 0; i < taskNum; ++i)
        {
            // no more commands for current task
            if (commandOfTask[i].empty())
                continue;

            cmd = commandOfTask[i].front();
            switch (s_mapActivity[cmd.a])
            {
                // init claimList and resrcLis
                case INITIATE:
                    taskList[i].claimList[cmd.c-1] = cmd.d;
                    taskList[i].resrcList[cmd.c-1] = 0;
                    taskList[i].takenTime++;

                    commandOfTask[i].pop();
                    break;

                case REQUEST:
                    // if total resource of type cmd.c-1 is greater than requested
                    if (cmd.d <= totalResourceList[cmd.c-1])
                    {
                        totalResourceList[cmd.c-1] -= cmd.d;
                        taskList[i].resrcList[cmd.c-1] += cmd.d;
                        taskList[i].takenTime++;
                        commandOfTask[i].pop();
                    }
                    else
                    {
                        // not pop out the command, keep waiting
                        taskList[i].waitingTime++;
                        waitingCnt++;
                    }
                    break;

                // release resource back to total list
                case RELEASE:
                    releaseResourceList[cmd.c-1] += cmd.d;
                    taskList[i].resrcList[cmd.c-1] -= cmd.d;
                    taskList[i].takenTime++;
                    commandOfTask[i].pop();
                    break;

                case COMPUTE:
                    commandOfTask[i].front().c--;
                    if (commandOfTask[i].front().c <= 0)
                        commandOfTask[i].pop();

                    taskList[i].takenTime++;
                    break;

                // clear commands and release its resources
                case TERMINATE:
                    releaseResourceList = taskList[i].resrcList;
                    commandOfTask[i] = queue<Command>();
                    terminateCnt++;
                    break;

                default:
                    cout << "Error. Unrecognized activity: " << cmd.a << endl;
            }
        }   

        // all tasks are waiting, which means deadlock
        if (waitingCnt >= taskNum  - terminateCnt)
        {
            // find the first non-terminated task to abort
            for (int i = 0; i < taskNum; i++)
            {
                // skip terminated tasks
                if (commandOfTask[i].empty())
                    continue;

                // clear commands and release its resources
                releaseResourceList = taskList[i].resrcList;
                commandOfTask[i] = queue<Command>();
            }
        }

        for (int i = 0; i < totalResourceList.size(); i++)
        {
            totalResourceList[i] += releaseResourceList[i];
        }

        cycle++; 
    }

    return taskList;
}

// vector<Task> bankerAlgorithm(
//     vector<Task> taskList, 
//     vector<queue<Command> > commandOfTask, 
//     vector<int> totalResourceList)
// {

// }


// main function for input data
int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        cout << "Error. Should run this program with a input file" << endl;
        return -1;
    }
    
    if (argc != 2)
    {
        cout << "Error. Too many arguments. Should only accept a input file" << endl;
        return -1;
    }

    const char* inputFilePath;

    inputFilePath = argv[1];
   
    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open())
    {
        cout << "Error. File " << inputFilePath << " does not exists." << endl;
        return -1;
    }

    // map each activity to an enum value
    initMap();

    int taskNum, resourceNum, unitNum;
    vector<Task> taskList;
    vector<queue<Command> > commandOfTask;
    vector<int> totalResourceList;

    // input the number of tasks, the number of resource types, followed
    // by the number of units present of each resource type.
    inputFile >> taskNum >> resourceNum;

    // init each task's values
    Task task;
    task.claimList.resize(resourceNum);
    task.resrcList.resize(resourceNum);
    task.takenTime = 0;
    task.waitingTime = 0;
    for (int i = 0; i < taskNum; i++) {
        task.taskNumber = i + 1;
        taskList.push_back(task);
    }

    commandOfTask.resize(taskNum);
    while (resourceNum--)
    {
        inputFile >> unitNum; 
        totalResourceList.push_back(unitNum);
    }

    // input each line of command
    Command cmd;
    while (!inputFile.eof())
    {
        inputFile >> cmd.a;
        if (s_mapActivity.find(cmd.a) == s_mapActivity.end())
        {
            cout << "Error. Unrecognized activity: " << cmd.a << endl;
            return -1;
        }

        inputFile >> cmd.b >> cmd.c >> cmd.d;

        // use task-number-1 as index to access a task's related commands
        commandOfTask[cmd.b-1].push(cmd);
    }

    inputFile.close();

    // testInput(taskList, commandOfTask, totalResourceList);
    vector<Task> result = optimisticResourceManager(taskList, commandOfTask, totalResourceList);
    cout << endl << "    FIFO    " << endl;
    output(result);
    return 0;
}

void output(vector<Task> taskList)
{
    int totalTakenTime = 0;
    int totalWaitingTime = 0;

    for (int i = 0; i < taskList.size(); i++)
    {
        totalTakenTime += taskList[i].takenTime;
        totalWaitingTime += taskList[i].waitingTime;

        printf("Task %d", taskList[i].taskNumber);
        if (taskList[i].takenTime >= 0)
        {
            printf("%5d%5d", taskList[i].takenTime, taskList[i].waitingTime);
            printf("%5d%%\n", 100 * taskList[i].waitingTime / (taskList[i].waitingTime + taskList[i].takenTime));
        }
        else
            cout << "     aborted" << endl;
    }

    printf("total %5d%5d", totalTakenTime, totalWaitingTime);
    printf("%5d%%\n\n", 100 * totalWaitingTime / (totalWaitingTime + totalTakenTime));
}

// generates input data for testing
void testInput(
    vector<Task> taskList, 
    vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList)
{
    cout << endl;
    for (vector<Task>::iterator it =taskList.begin(); it !=taskList.end(); it++)
    {
        cout << "Task: " << it->taskNumber << endl;
        cout << "    Claim:";
        for (int i = 0; i < it->claimList.size(); i++)
        {
            cout << " (" << i+1 << ": " << it->claimList[i] << ")";
        }
        cout << endl;
        cout << "    TakenTime: " << it->takenTime << endl;
        cout << "    WaitingTime: " << it->waitingTime << endl;
    }
    cout << endl;

    cout << "--------------------Commands--------------------" << endl;

    for (vector<queue<Command> >::iterator it =commandOfTask.begin(); it !=commandOfTask.end(); it++)
    {
        Command cmd;
        while (!it->empty())
        {
            cmd = it->front();
            cout << cmd.a << " " << cmd.b << " " << cmd.c << " " << cmd.d << endl;
            it->pop();
        }

        cout << endl << endl;
    }

    cout << "Total resources we have: ";
    for (int i = 0; i < totalResourceList.size(); i++)
    {
        cout << " (" << i+1 << ": " << totalResourceList[i] << ")";
    }
    cout << endl;
}
