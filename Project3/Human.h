//
// Created by Xuan Zhai on 2021/4/10.
//

#ifndef PROJECT3_HUMAN_H
#define PROJECT3_HUMAN_H
#include <cstdlib>
#include <vector>
#include <ctime>

class Human {

private:
    int ID = 0;
    int Type = 0;
    int Index = 0;                  // Index number in the vector

public:
    std::vector<int> wlist;             // A list of potential request
    std::vector<int> vlist;             // A list of numbers of visited time for each potential requested item
    std::vector<int> hlist;             // What items he is holding right now
    Human();
    ~Human();
    void SetID(int);
    void SetType(int);
    void SetIndex(int);
    int GetID();
    int GetIndex();
    bool AllVisited();
    bool isholding(int);
    void Incvlist(int);
};


#endif //PROJECT3_HUMAN_H
