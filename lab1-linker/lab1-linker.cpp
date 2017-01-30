#include <iostream>
#include <vector>
#include <map>
#include <set>
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
    int testcase = 1;
    while(cin >> n && n > 0) {    
        cout << "-----------------------------------------------" << endl;
        cout << "                 Test Case # " << testcase++ << endl << endl;

        map<string, int> symbolMap;
        set<string> symbolUsed;

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
                else
                {
                    cout << "Error: The symbol '" << temp.symbol 
                    << "‘ was multiply defined, first value used" << endl; 
                }
            }

            // unique set of location, check for multiple usage of symbols in the same instruction
            set<int> uniqueLocs;

            // input use lists
            cin >> cnt;
            while(cnt--)
            {
                UseList temp;
                int location;

                cin >> temp.symbol;

                while(cin >> location && location != -1)
                {
                    // new location, add to the unique set and allow the usage
                    if (uniqueLocs.find(location) == uniqueLocs.end())
                    {
                        uniqueLocs.insert(location);
                        temp.locations.push_back(location);
                    }
                    else
                    {
                        cout << "Error: Multiple variables used in instruction " << location << ". Ignore usage: '" << temp.symbol << "'." << endl;
                    }
                }

                // the symbol has been used in some valid locations, admit its usage
                // if (!temp.locations.empty())
                // {
                    symbolUsed.insert(temp.symbol);
                    inputSet.uselists.push_back(temp);
                // }

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

        // since the symbols has valid usage, check their definition
        for (set<string>::iterator it=symbolUsed.begin(); it!=symbolUsed.end(); ++it)
        {
            string symbol = *it;
            // if not defined
            if (symbolMap.find(symbol) == symbolMap.end())
            {
                symbolMap[symbol] = 0;
                cout << "Error: The symbol ‘" << symbol << "’ was used but not defined. It has been given the value 0" << endl;
            }
        }


        // defined but not used
        for (map<string, int>::iterator it = symbolMap.begin(); it != symbolMap.end(); ++it)
        {
            if (symbolUsed.find(it->first) == symbolUsed.end())
            {
                cout << "Warning: The symbol ‘" << it->first << "’ was defined but never used." << endl;
            }
        }

        // the first pass
        int baseAddr = 0;
        for (vector<InputSet>::iterator i = inputSetList.begin(); i != inputSetList.end(); ++i)
        {
            int moduleSize = i->insList.size();
            for (vector<DefSym>::iterator j = i->defList.begin(); j != i->defList.end(); ++j)
            {
                symbolMap[j->symbol] += baseAddr;

                if (j->location >= moduleSize)
                {
                    j->location = 0;
                    symbolMap[j->symbol] = baseAddr;
                    cout << "Error. Definition of '" << j->symbol 
                    << "' exceeds module size, first word in module used." << endl;
                }
            }
            baseAddr += moduleSize;
        }

        cout << "-----------------------------------------------" << endl;
        cout << "                 Symbol Table" << endl;

        // print out all elements in the symbol table
        for (map<string,int>::iterator it=symbolMap.begin(); it!=symbolMap.end(); ++it)
            cout << it->first << " = " << it->second << endl;

        cout << endl;
        cout << "-----------------------------------------------" << endl;
        cout << "                  Memory Map" << endl;

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
                ostringstream s;
                s << output1[j->location] << j->symbol << ":";
                output1[j->location] = s.str();
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

                        int tempAddr = i->insList[j->locations[k]].addr;
                        tempAddr /= 1000;
                        tempAddr *= 1000;
                        i->insList[j->locations[k]].addr = tempAddr + symbolMap[j->symbol];
                    }
                    else
                    {
                        cout << "Error: Use of '" << j->symbol << "' with an address of " << j->locations[k]
                        << "exceeds module size. Use ignored." << endl;
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
                        if (j->addr % 1000 >= 200)
                        {
                            j->addr = j->addr/1000*1000;
                            s << j->addr << " Error: Absolute address exceeds machine size; zero used." << endl;
                        }
                        else
                        {
                            s << j->addr;
                        }
 
                        result = s.str();
                        break;
                    case 'R':
                        if (j->addr % 1000 >= moduleSize)
                        {
                            j->addr = j->addr/1000*1000;
                            s << j->addr << " Error: Relative address exceeds machine size; zero used." << endl;
                        }
                        else
                        {
                            s << j->addr + baseAddr << " = " << j->addr << "+" << baseAddr;
                        }
 
                        result = s.str();
                        break;
                    case 'E':
                        s << j->addr;
                        result = s.str();
                        break;
                    default:   
                        cout << "Unknown instruction: " << j->c << endl;             
                }

                printf("%-12s %-17s %-s\n", output1[lineNum].c_str(), output2[lineNum].c_str(), result.c_str());
            }

            baseAddr += moduleSize;
        }
        cout << endl << endl;
    }

    return 0;
}
