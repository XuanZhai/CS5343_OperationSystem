//
// Created by Xuan Zhai on 2021/4/13, reference: https://www.geeksforgeeks.org/detect-cycle-in-a-graph/
//

#ifndef PROJECT3_GRAPH_H
#define PROJECT3_GRAPH_H
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

class Graph
{
    int V;                                      // No. of vertices
    int Rvertics,Wvertics;                  // How many read and how many writer
    std::list<int> *adj;                    // Pointer to an array containing adjacency lists
    bool isCyclicUtil(int v, bool visited[], bool *rs);  // used by isCyclic()
public:
    Graph(int V);                           // Constructor
    Graph(const Graph&);
    ~Graph();
    Graph& operator=(const Graph&);
    void setGraph(int,int);             //Set Rvertics and Wvertics
    void addEdge(int v, int w);         // to add an edge to graph
    void deleteEdge(int v, int w);      // delete an edge in the graph
    void changeEdge(int v, int w);      // Change the direction of the edge, also change the one that the target is holding. i.e. 1->2->3 will be 1<-2<-3
    bool isCyclic();                    // returns true if there is a cycle in this graph
    bool hasWrite(int);                 // Check if there is a writer relate to that vertice.
    void printGraph();
    void clearEdge(int);                // Remove a vertice and all its edges.
};

#endif //PROJECT3_GRAPH_H
