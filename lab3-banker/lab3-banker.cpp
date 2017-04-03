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

#ifndef DEBUGMODE
#define DEBUGMODE false
#endif

#ifndef D
#define DEBUG(x) if (DEBUGMODE) {x}
#endif

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
    // vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList);

void output(vector<Task> taskList1, vector<Task> taskList2);

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

vector<Task> optimisticResourceManager(
    vector<Task> taskList, 
    vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList)
{
    int cycle = 0;
    int taskNum = taskList.size();
    queue<int> pendingTasks;
    queue<int> runningTasks;

    // push all task-number into queue
    for (int i = 0; i < taskNum; ++i)
        runningTasks.push(i);

    // run until no tasks is either pending or running (all tasks terminate)
    while (!pendingTasks.empty() || !runningTasks.empty())
    {
        Command cmd;
        // resources to be released after current cycle
        vector<int> releaseResourceList(totalResourceList.size(), 0);

        // resources to be required after current cycle
        vector<int> requiredResourceList(totalResourceList.size(), 0);

        queue<int> tempPendingTasks;
        queue<int> tempRunningTasks;

        DEBUG(cout << "During " << cycle << "-" << cycle+1 << endl;)

        // run all tasks in both queue
        while (!pendingTasks.empty() || !runningTasks.empty())
        {
            // index of task
            int i = 0;

            // check pending queue first
            if (!pendingTasks.empty())
            {
                i = pendingTasks.front();
                pendingTasks.pop();
            }
            else
            {
                i = runningTasks.front();
                runningTasks.pop();
            }

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

                    tempRunningTasks.push(i);

                    DEBUG(cout << "task " << i+1 << " completes its initiate" << endl;)
                    break;

                case REQUEST:
                    // if total resource of type cmd.c-1 is greater than requested
                    taskList[i].takenTime++;
                    if (cmd.d <= totalResourceList[cmd.c-1])
                    {
                        totalResourceList[cmd.c-1] -= cmd.d;
                        taskList[i].resrcList[cmd.c-1] += cmd.d;
                        
                        commandOfTask[i].pop();
                        
                        tempRunningTasks.push(i);

                        DEBUG(cout << "task " << i+1 << " request is granted" << endl;)
                    }
                    else
                    {
                        // indicate how many resources required
                        requiredResourceList[cmd.c-1] += cmd.d;

                        // not pop out the command, keep waiting
                        taskList[i].waitingTime++;

                        tempPendingTasks.push(i);

                        DEBUG(cout << "task " << i+1 << " request cannot be granted" << endl;)
                    }
                    break;

                // release resource back to total list
                case RELEASE:
                    releaseResourceList[cmd.c-1] += cmd.d;
                    taskList[i].resrcList[cmd.c-1] -= cmd.d;
                    taskList[i].takenTime++;
                    commandOfTask[i].pop();
                    tempRunningTasks.push(i);
                    
                    DEBUG(cout << "task " << i+1 << " release " << cmd.d << " units of type " << cmd.c << endl;)
                    break;

                case COMPUTE:
                    commandOfTask[i].front().c--;
                    DEBUG(cout << "task " << i+1 << " computing, " << commandOfTask[i].front().c << " cycles left." << endl;)

                    if (commandOfTask[i].front().c <= 0)
                        commandOfTask[i].pop();

                    taskList[i].takenTime++;

                    tempRunningTasks.push(i);
                    break;

                // clear commands and release its resources
                case TERMINATE:
                    releaseResourceList = taskList[i].resrcList;
                    commandOfTask[i] = queue<Command>();

                    DEBUG(cout << "task " << i+1 << " release all resources and terminate" << endl;)
                    break;

                default:
                    cout << "Error. Unrecognized activity: " << cmd.a << endl;
            }
        }   

        // cout << "remaining resource at cycle: " << cycle+1 << endl;
        for (int i = 0; i < totalResourceList.size(); i++)
        {
            totalResourceList[i] += releaseResourceList[i];
            // cout << " (" << i+1 << ": " << totalResourceList[i] << ")";
        }

        // if all tasks are waiting, which means deadlock
        if (!tempPendingTasks.empty() && tempRunningTasks.empty())
        {
            for (int i = 0; i < taskNum; i++)
            {
                // find the non-terminated task with minimum task-number to abort
                if (!commandOfTask[i].empty())
                {
                    // this must be a request command
                    Command cmd = commandOfTask[i].front();

                    // clear commands and release its resources, reduce its required resources
                    for (int j = 0; j < totalResourceList.size(); j++)
                        totalResourceList[j] += taskList[i].resrcList[j];
                    requiredResourceList[cmd.c-1] -= cmd.d;

                    commandOfTask[i] = queue<Command>();

                    // set takenTime and waitingTime to 0 to indicate abort
                    taskList[i].takenTime = 0;
                    taskList[i].waitingTime = 0;

                    DEBUG(cout << "task " << i+1 << " aborted" << endl;)
                }

                bool isEnough = false;
                // compare totalResourceList and requiredResourceList to see if it can satisfied one task to run
                for (int j = 0; j < totalResourceList.size(); j++)
                    if (totalResourceList[j] >= requiredResourceList[j])
                    {
                        isEnough = true;
                        break;
                    }

                if (isEnough)
                    break;
            }

            // go through pending queue and remove aborted tasks
            while (!tempPendingTasks.empty())
            {
                // if task i+1 not terminated, add to the pending queue
                int i = tempPendingTasks.front();
                tempPendingTasks.pop();

                if (!commandOfTask[i].empty())
                    pendingTasks.push(i);                
            }

        }
        else
        {
            runningTasks = tempRunningTasks;
            pendingTasks = tempPendingTasks;
        }

        cycle++; 
    }

    return taskList;
}

// check if safe after satisfying the request command
bool isSafe(vector<Task> taskList, vector<int> totalResourceList, Command cmd)
{
    bool safe = true;
    totalResourceList[cmd.c-1] -= cmd.d;
    taskList[cmd.b-1].resrcList[cmd.c-1] += cmd.d;

    DEBUG(testInput(taskList, totalResourceList);)

    vector<bool> taskFinished(taskList.size(), false);
    for (int i = 0; i < taskList.size(); i++)
    {
        // DEBUG(cout << "Try task " << i+1 << endl;)
        if (taskFinished[i])
            continue;

        bool isEnough = true;
        for (int j = 0; j < totalResourceList.size(); j++)
        {
            // if system cannot satisfy resource j+1 of task i+1
            if (totalResourceList[j] < taskList[i].claimList[j] - taskList[i].resrcList[j])
            {
                isEnough = false;
                break;
            }
        }

        if (isEnough)
        {
            // add to total, finish task i+1
            for (int j = 0; j < totalResourceList.size(); j++)
            {
                totalResourceList[j] += taskList[i].resrcList[j];
                taskList[i].resrcList[j] = 0;
                // DEBUG(cout << " (" << j+1 << ": " << totalResourceList[j] << ") ";)
            }
            taskFinished[i] = true;
            // reset index to begin from the first task
            i = -1;
        }
    }

    int finishCnt = 0;
    for (int i = 0; i < taskFinished.size(); i++)
        if (taskFinished[i])
            finishCnt++;

    if (finishCnt == taskList.size())
        return true;

    return false;
}

vector<Task> bankerAlgorithm(
    vector<Task> taskList, 
    vector<queue<Command> > commandOfTask, 
    vector<int> totalResourceList)
{
    int cycle = 0;
    int taskNum = taskList.size();
    queue<int> pendingTasks;
    queue<int> runningTasks;

    // push all task-number into queue
    for (int i = 0; i < taskNum; ++i)
        runningTasks.push(i);

    // run until no tasks is either pending or running (all tasks terminate)
    while (!pendingTasks.empty() || !runningTasks.empty())
    {
        Command cmd;
        // resources to be released after current cycle
        vector<int> releaseResourceList(totalResourceList.size(), 0);

        // resources to be required after current cycle
        vector<int> requiredResourceList(totalResourceList.size(), 0);

        queue<int> tempPendingTasks;
        queue<int> tempRunningTasks;

        DEBUG(cout << "During " << cycle << "-" << cycle+1 << endl;)

        // run all tasks in both queue
        while (!pendingTasks.empty() || !runningTasks.empty())
        {
            // index of task
            int i = 0;

            // check pending queue first
            if (!pendingTasks.empty())
            {
                i = pendingTasks.front();
                pendingTasks.pop();
            }
            else
            {
                i = runningTasks.front();
                runningTasks.pop();
            }

            // no more commands for current task
            if (commandOfTask[i].empty())
                continue;

            cmd = commandOfTask[i].front();
            switch (s_mapActivity[cmd.a])
            {
                // init claimList and resrcLis
                case INITIATE:
                    // if claimed resources is more than total resources, abort this task
                    if (totalResourceList[cmd.c-1] < cmd.d)
                    {
                        // clear commands and release its resources
                        releaseResourceList = taskList[i].resrcList;
                        commandOfTask[i] = queue<Command>();

                        // set takenTime and waitingTime to 0 to indicate abort
                        taskList[i].takenTime = 0;
                        taskList[i].waitingTime = 0;

                        printf("Banker aborts task %d before run begins:\n", i+1);
                        printf("    claim for resourse %d (%d) exceeds number of units present (%d)\n", cmd.c, cmd.d, totalResourceList[cmd.c-1]);
                    }
                    else
                    {
                        taskList[i].claimList[cmd.c-1] = cmd.d;
                        taskList[i].resrcList[cmd.c-1] = 0;
                        taskList[i].takenTime++;

                        commandOfTask[i].pop();

                        tempRunningTasks.push(i);

                        DEBUG(cout << "task " << i+1 << " completes its initiate" << endl;)
                    }
                    break;

                case REQUEST:
                    taskList[i].takenTime++;

                    // totoal requested resources is more than claimed, aborted
                    if (taskList[i].resrcList[cmd.c-1] + cmd.d > taskList[i].claimList[cmd.c-1])
                    {
                        // clear commands and release its resources
                        releaseResourceList = taskList[i].resrcList;
                        commandOfTask[i] = queue<Command>();

                        // set takenTime and waitingTime to 0 to indicate abort
                        taskList[i].takenTime = 0;
                        taskList[i].waitingTime = 0;

                        printf("During cycle %d-%d of Banker's algorithms\n", cycle, cycle+1);
                        printf("    Task %d's request exceeds its claim; aborted;\n", i+1);
                    }
                    // has enough resources and is safe after granted
                    else if (cmd.d <= totalResourceList[cmd.c-1] && isSafe(taskList, totalResourceList, cmd))
                    {
                        // grant resources
                        totalResourceList[cmd.c-1] -= cmd.d;
                        taskList[i].resrcList[cmd.c-1] += cmd.d;

                        commandOfTask[i].pop();
                        
                        tempRunningTasks.push(i);

                        DEBUG(cout << "task " << i+1 << " request is granted" << endl;)
                    }
                    // not safe or not enough resources to grant, let the process wait
                    else
                    {
                        // indicate how many resources required
                        requiredResourceList[cmd.c-1] += cmd.d;

                        // not pop out the command, keep waiting
                        taskList[i].waitingTime++;

                        tempPendingTasks.push(i);

                        DEBUG(cout << "task " << i+1 << " request cannot be granted" << endl;)
                    }
                    break;

                // release resource back to total list
                case RELEASE:
                    releaseResourceList[cmd.c-1] += cmd.d;
                    taskList[i].resrcList[cmd.c-1] -= cmd.d;
                    taskList[i].takenTime++;
                    commandOfTask[i].pop();
                    tempRunningTasks.push(i);
                    
                    DEBUG(cout << "task " << i+1 << " release " << cmd.d << " units of type " << cmd.c << endl;)
                    break;

                case COMPUTE:
                    commandOfTask[i].front().c--;
                    DEBUG(cout << "task " << i+1 << " computing, " << commandOfTask[i].front().c << " cycles left." << endl;)

                    if (commandOfTask[i].front().c <= 0)
                        commandOfTask[i].pop();

                    taskList[i].takenTime++;

                    tempRunningTasks.push(i);
                    break;

                // clear commands and release its resources
                case TERMINATE:
                    releaseResourceList = taskList[i].resrcList;
                    commandOfTask[i] = queue<Command>();

                    DEBUG(cout << "task " << i+1 << " release all resources and terminate" << endl;)
                    break;

                default:
                    cout << "Error. Unrecognized activity: " << cmd.a << endl;
            }
        }   

        // cout << "remaining resource at cycle: " << cycle+1 << endl;
        for (int i = 0; i < totalResourceList.size(); i++)
        {
            totalResourceList[i] += releaseResourceList[i];
            // cout << " (" << i+1 << ": " << totalResourceList[i] << ")";
        }

        runningTasks = tempRunningTasks;
        pendingTasks = tempPendingTasks;

        cycle++; 
    }

    return taskList;
}

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
    vector<Task> result1 = optimisticResourceManager(taskList, commandOfTask, totalResourceList);
    vector<Task> result2 = bankerAlgorithm(taskList, commandOfTask, totalResourceList);
    
    cout << endl << "  \t\t\tFIFO\t\t\t\t\t\t\tBANKER'S" << endl;
    output(result1, result2);
    return 0;
}

void output(vector<Task> taskList1, vector<Task> taskList2)
{
    int totalTakenTime1 = 0;
    int totalWaitingTime1 = 0;
    int totalTakenTime2 = 0;
    int totalWaitingTime2 = 0;

    for (int i = 0; i < taskList1.size(); i++)
    {
        totalTakenTime1 += taskList1[i].takenTime;
        totalWaitingTime1 += taskList1[i].waitingTime;

        totalTakenTime2 += taskList2[i].takenTime;
        totalWaitingTime2 += taskList2[i].waitingTime;

        printf("     Task %d", taskList1[i].taskNumber);
        if (taskList1[i].takenTime > 0)
        {
            printf("%7d%4d", taskList1[i].takenTime, taskList1[i].waitingTime);
            printf("%4d%%", int(0.5 + 100.0 * taskList1[i].waitingTime / taskList1[i].takenTime));
        }
        else
            printf("%16s", "aborted");


        cout << "\t\t";

        printf("\t\tTask %d", taskList2[i].taskNumber);
        if (taskList2[i].takenTime > 0)
        {
            printf("%7d%4d", taskList2[i].takenTime, taskList2[i].waitingTime);
            printf("%4d%%", int(0.5 + 100.0 * taskList2[i].waitingTime / taskList2[i].takenTime));
        }
        else
            cout << "     aborted";

        cout << endl;
    }

    printf("     total %7d%4d", totalTakenTime1, totalWaitingTime1);
    if (totalTakenTime1 > 0)
        printf("%4d%%", int(100.0 * totalWaitingTime1 / totalTakenTime1 + 0.5));
    else
        printf("%4d%%", 0);

    cout << "\t\t";

    printf("\t\ttotal %7d%4d", totalTakenTime2, totalWaitingTime2);
    if (totalTakenTime2 > 0)
        printf("%4d%%", int(100.0 * totalWaitingTime2 / totalTakenTime2 + 0.5));
    else
        printf("%4d%%", 0);

    cout << endl << endl;
}

// generates input data for testing
void testInput(
    vector<Task> taskList, 
    // vector<queue<Command> > commandOfTask, 
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
        cout << endl;cout << "    Have:";
        for (int i = 0; i < it->resrcList.size(); i++)
        {
            cout << " (" << i+1 << ": " << it->resrcList[i] << ")";
        }
        cout << endl;
        // cout << "    TakenTime: " << it->takenTime << endl;
        // cout << "    WaitingTime: " << it->waitingTime << endl;
    }
    cout << endl;

    // cout << "--------------------Commands--------------------" << endl;
    // for (vector<queue<Command> >::iterator it =commandOfTask.begin(); it !=commandOfTask.end(); it++)
    // {
    //     Command cmd;
    //     while (!it->empty())
    //     {
    //         cmd = it->front();
    //         cout << cmd.a << " " << cmd.b << " " << cmd.c << " " << cmd.d << endl;
    //         it->pop();
    //     }

    //     cout << endl << endl;
    // }

    cout << "Total resources we have: ";
    for (int i = 0; i < totalResourceList.size(); i++)
    {
        cout << " (" << i+1 << ": " << totalResourceList[i] << ")";
    }
    cout << endl;
}
