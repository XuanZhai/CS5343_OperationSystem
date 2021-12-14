//
// Created by Xuan Zhai on 2021/4/10.
//

#include "Human.h"

using namespace std;

Human::Human(){
    ID = -1;
    Type = -1;
}

Human::~Human(){                // Destructure
    ID = -1;
    Type = -1;
    wlist.clear();
    vlist.clear();
}

void Human::SetID(int nID){
    ID = nID;
}

void Human::SetType(int nType){
    Type = nType;
}

void Human::SetIndex(int nIndex) {
        Index = nIndex;
}

int Human::GetID(){
    return ID;
}

int Human::GetIndex() {
    return Index;
}

bool Human::AllVisited(){                       // Check if every item that the reader needed has been read at least once
    for(int i = 0; i < vlist.size(); i++){
        if(vlist.at(i) < 1)
            return false;
    }
    return true;
}


bool Human::isholding(int index) {              // Check if the person is holding that item
    for(int i = 0; i < hlist.size(); i++){
        if(index == hlist.at(i)){
            return true;
        }
    }
    return false;
}


void Human::Incvlist(int index){                // If just visited, increment the visited time in the visited list
    for(int i = 0; i < wlist.size(); i++){
        if(index == wlist.at(i)){
            vlist.at(i)++;
            break;
        }
    }
}