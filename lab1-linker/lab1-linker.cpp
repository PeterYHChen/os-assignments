#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <sstream>

using namespace std;

struct Instruction
{
    char c;
    int addr;
};

struct UseList
{
    string symbol;
    vector<int> locations;  
};

struct DefSym
{
    string symbol;
    int location;
};

struct InputSet
{
    vector<DefSym> defList;
    vector<UseList> uselists;
    vector<Instruction> insList;
};

int main()
{
    int n;
    cin >> n;

    map<string, int> symbolMap;

    vector<InputSet> inputSetList;

    while(n--)
    {
        int cnt;
        InputSet inputSet;

        // input define symbols
        cin >> cnt;
        while(cnt--)
        {
            DefSym temp;
            cin >> temp.symbol >> temp.location;
            if (symbolMap.find(temp.symbol) == symbolMap.end())
            {
                symbolMap[temp.symbol] = temp.location;
                inputSet.defList.push_back(temp);
            }
        }

        // input use lists
        cin >> cnt;
        while(cnt--)
        {
            UseList temp;
            int location;

            cin >> temp.symbol;
            while(cin >> location && location != -1)
            {
                temp.locations.push_back(location);
            }

            inputSet.uselists.push_back(temp);
        }

        // input instruction list
        cin >> cnt;
        while(cnt--)
        {
            Instruction temp;
            cin >> temp.c >> temp.addr;
            inputSet.insList.push_back(temp);
        }

        inputSetList.push_back(inputSet);
    }

    cout << "Symbol Table" << endl;
    // the first pass
    int baseAddr = 0;
    for (vector<InputSet>::iterator i = inputSetList.begin(); i != inputSetList.end(); ++i)
    {
        for (vector<DefSym>::iterator j = i->defList.begin(); j != i->defList.end(); ++j)
        {
            symbolMap[j->symbol] += baseAddr;           
            cout << j->symbol<< " = " << symbolMap[j->symbol] << endl;
        }
        baseAddr += i->insList.size();
    }

    cout << endl << "Memory Map" << endl;

    // the second pass
    baseAddr = 0;
    for (vector<InputSet>::iterator i = inputSetList.begin(); i != inputSetList.end(); ++i)
    {
        cout << "+" << baseAddr << endl;
        vector<string> output1, output2, output3;

        int moduleSize = i->insList.size();
        for (vector<Instruction>::iterator j = i->insList.begin(); j != i->insList.end(); ++j)
        {
            int lineNum = j - i->insList.begin();
            ostringstream s;
            s << lineNum << ": ";
            output1.push_back(s.str());
            s.str("");
            s << j->c << " " << j->addr;
            output2.push_back(s.str());
        }

        // add define list content to output1
        for (vector<DefSym>::iterator j = i->defList.begin(); j != i->defList.end(); ++j)
        {
            // location within module
            if (j->location < moduleSize)
            {
                ostringstream s;
                s << output1[j->location] << j->symbol << ":";
                output1[j->location] = s.str();
            }
        }

        // add use list content to output2
        for (vector<UseList>::iterator j = i->uselists.begin(); j != i->uselists.end(); ++j)
        {
            for (int k = 0; k < j->locations.size(); k++)
            {
                // location within module
                if (j->locations[k] < moduleSize)
                {
                    ostringstream s;
                    s << output2[j->locations[k]] << " ->" << j->symbol;
                    output2[j->locations[k]] = s.str();

                    i->insList[j->locations[k]].addr += symbolMap[j->symbol];
                }
            }
        }

        // output address changes
        for (vector<Instruction>::iterator j = i->insList.begin(); j != i->insList.end(); ++j)
        {
            int lineNum = j - i->insList.begin();
            string result;
            ostringstream s;
            switch(j->c)
            {
                case 'I':
                    s << j->addr;
                    result = s.str();
                    break;
                case 'A':
                    s << j->addr;
                    result = s.str();
                    break;
                case 'R':
                    s << j->addr << "+" << baseAddr << " = " << j->addr + baseAddr;
                    result = s.str();
                    break;
                case 'E':
                    s << j->addr;
                    result = s.str();
                    break;
                default:   
                    cout << "Unknown instruction" << endl;             
            }

            printf("%-12s %-17s %15s\n", output1[lineNum].c_str(), output2[lineNum].c_str(), result.c_str());
        }

        baseAddr += moduleSize;
    }

    return 0;
}
