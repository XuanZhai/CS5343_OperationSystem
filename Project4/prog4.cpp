#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

using namespace std;


int FindLongest(std::unordered_map<int, std::vector<int>>& numberlist, std::vector<int>& framelist, std::vector<int>& pagelist, int current){
    int longestdist = 0;                // The longest distance between the current pos and the next appear pos.
    int longindex = 0;                  // Which number in the frame table has the longest distance.

    for(int i = 0; i < framelist.size(); i++){                  // For each frame in the table, loop through the location it appears
        bool haslarger = false;
        for(int j = 0; j < numberlist[pagelist.at(framelist.at(i))].size(); j++){
            if(current < numberlist[pagelist.at(framelist.at(i))].at(j)){               // If there's one after the current position
                if(numberlist[pagelist.at(framelist.at(i))].at(j) - current > longestdist){     // If distance is longer than the current largest
                    longestdist = numberlist[pagelist.at(framelist.at(i))].at(j) - current;
                    longindex = i;                          // Update the largest
                }
                numberlist[pagelist.at(framelist.at(i))].erase(numberlist[pagelist.at(framelist.at(i))].begin(), numberlist[pagelist.at(framelist.at(i))].begin() + j);
                haslarger = true;
                break;
            }
        }
        if(!haslarger) {
            numberlist[pagelist.at(framelist.at(i))].clear();
            return i;       // If that page does not have a location after the current, it means that that frame will not appear again.
        }
    }
    return longindex;
}



int Optimal(vector<int>& pagelist, int f, bool toprint = false){
    std::unordered_map<int, std::vector<int>> numberlist;           // A hash table to store the number and the index
    int nPageFault = 0;                                             // it showed.

    for(int i = 0; i < pagelist.size(); i++)
        numberlist[pagelist.at(i)].push_back(i);                    // Fill the hash table

    std::vector<int> framelist;
    for(int i = 0; i < f; i++){                                     // Fill the frame table
        framelist.push_back(i);
        nPageFault++;
        if(toprint)
            printf("Page %d accessed. Page fault.\n", pagelist.at(i));
    }

    for(int i = f; i < pagelist.size(); i++){
        bool exist = false;
        for(int j = 0; j < f; j++){                             // Find if the page exists in the table
            if(pagelist.at(framelist.at(j)) == pagelist.at(i)){
                exist = true;
                if(toprint)
                    printf("Page %d accessed.\n", pagelist.at(i));
                break;
            }
        }

        if(!exist){                                             // If not exist
            int result = FindLongest(numberlist, framelist, pagelist, i); // Find the target to be replaced
            if(toprint)
                printf("Page %d accessed. Page %d replaced.\n", pagelist.at(i), pagelist.at(framelist.at(result)));
            framelist.at(result) = i;           // Do the replacement
            nPageFault++;
        }

    }
    return nPageFault;
}



int LRU(vector<int>& pagelist, int f, bool toprint = false){
    std::vector<int> framelist;                         // A list of current frame
    int nPageFault = 0;

    for(int i = 0; i < f; i++){                         // Fill the table
        framelist.push_back(i);
        nPageFault++;
        if(toprint)
            printf("Page %d accessed. Page fault.\n", pagelist.at(i));
    }


    for(int i = f; i < pagelist.size(); i++){           // Loop through the rest
        bool exist = false;
        for(int j = 0; j < f; j++){                     // Find if it is already in the table
            if(pagelist.at(framelist.at(j)) == pagelist.at(i)){     // If find
                framelist.erase(framelist.begin() + j);             // update the index number and move it to the bottom.
                framelist.push_back(i);                             // We want to make sure it's sorted.
                exist = true;
                if(toprint)
                    printf("Page %d accessed.\n", pagelist.at(i));
                break;
            }
        }

        if(!exist){
            if(toprint)
                printf("Page %d accessed. Page %d replaced.\n", pagelist.at(i), pagelist.at(framelist.at(0)));
            framelist.erase(framelist.begin());             // Replace the first one. (the one with the lowest index)
            framelist.push_back(i);
            nPageFault++;
        }
    }
    return nPageFault;
}


int main(int argc, char **argv)
{

    ifstream ifile(argv[1]);
    int numofstring;

    ifile >> numofstring;
    vector<int> strings[numofstring];
    int frame[numofstring];

    for (int i = 0; i < numofstring; i++)
    {
        int n;
        ifile >> frame[i] >> n;
        for (int j = 0; j < n; j++)
        {
            int p;
            ifile >> p;
            strings[i].push_back(p);
        }
    }


    for (int i = 0; i < numofstring; i++)
    {
        int optValue = Optimal(strings[i], frame[i], true);
        cout << "======================================================" << endl;
        int LRUValue = LRU(strings[i], frame[i], true);
        cout << "String " << i+1 << " (";
        for (unsigned int j = 0; j < strings[i].size(); j++)
            cout << " " << strings[i][j];
        cout << ") : " << " # of pages for Opt/LRU " << optValue << " / " << LRUValue << endl;
    }

    return 0;
}