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

        if (s_mapActivity[cmd.a] == INITIATE) {
            taskList[cmd.b-1].claimList[cmd.c-1] = cmd.d;
        }
        // use task-number-1 as index to access a task's related commands
        commandOfTask[cmd.b-1].push(cmd);
    }

    inputFile.close();

    // testInput(taskList, commandOfTask, totalResourceList);
    optimisticResourceManager(taskList, commandOfTask, totalResourceList);
    return 0;
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
