//
// Created by Xuan Zhai on 2021/4/17.
//
#include "Graph.h"

Graph::Graph(int V)
{
    this->V = V;
    Rvertics = 0;
    Wvertics = 0;
    adj = new std::list<int>[V];
}

Graph::~Graph(){
    delete[] adj;
    V = 0;
    Rvertics = 0;
    Wvertics = 0;
}


Graph::Graph(const Graph& nGraph){              // Copy constructor
    this->V = nGraph.V;
    adj = new std::list<int>[V];
    for(int i = 0; i < V; i++){
        adj[i] = nGraph.adj[i];
    }
    Rvertics = nGraph.Rvertics;
    Wvertics = nGraph.Wvertics;
}


Graph& Graph::operator=(const Graph& nGraph){           // Overload assignment operator
    this->V = nGraph.V;
    adj = new std::list<int>[V];
    for(int i = 0; i < V; i++){
        adj[i] = nGraph.adj[i];
    }
    Rvertics = nGraph.Rvertics;
    Wvertics = nGraph.Wvertics;
    return *this;
}


void Graph::setGraph(int a,int b){
    Rvertics = a;
    Wvertics = b;
}


void Graph::addEdge(int v, int w)
{
    std::list<int>::iterator it;
    it = std::find (adj[v].begin(), adj[v].end(), w);           // Find if the edge is already in the graph

    if(it != adj[v].end()){
        return;
    }
    else{
        adj[v].push_back(w);                        // If no, add the edge
    }
}

void Graph::deleteEdge(int v, int w){
    std::list<int>::iterator it;
    it = std::find (adj[v].begin(), adj[v].end(), w);           // Find the target edge in the graph

    if(it != adj[v].end()){
        adj[v].erase(it);                           // Delete if it exists
    }
    else{
        return;
    }
}


void Graph::changeEdge(int s, int d){
    std::vector<int> temp;

    while (adj[s].size() != 0){
        int a = adj[s].back();
        adj[s].pop_back();
        adj[a].push_back(s);                // revert all the edges it towards, basically release all the release in the graph
    }

    deleteEdge(d,s);
    addEdge(s,d);                       // Change the direction of the edge
}


// This function is a variation of DFSUtil() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclicUtil(int v, bool visited[], bool *recStack)
{
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;

        // Recur for all the vertices adjacent to this vertex
        std::list<int>::iterator i;
        for(i = adj[v].begin(); i != adj[v].end(); ++i)
        {
            if ( !visited[*i] && isCyclicUtil(*i, visited, recStack) )
                return true;
            else if (recStack[*i])
                return true;
        }

    }
    recStack[v] = false;  // remove the vertex from recursion stack
    return false;
}

// Returns true if the graph contains a cycle, else false.
// This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclic()
{
    // Mark all the vertices as not visited and not part of recursion
    // stack
    bool *visited = new bool[V];
    bool *recStack = new bool[V];
    for(int i = 0; i < V; i++)
    {
        visited[i] = false;
        recStack[i] = false;
    }

    // Call the recursive helper function to detect cycle in different
    // DFS trees
    for(int i = 0; i < V; i++) {
        if (isCyclicUtil(i, visited, recStack)) {
            for(int j = 0; j < V; j++){
                if(recStack[j] == true && j < (Rvertics + Wvertics) && (j >= Rvertics)  ) {        // Different between reader and writer
                    delete[] visited;                                               // iscycle will also check if there's a writer in the cycle
                    delete[] recStack;                                              // If all the vertices are readers, it will accept that cus it won't cause a deadlock
                    return true;                                                    // even it has a cycle.
                }
            }
        }
    }
    delete[] visited;
    delete[] recStack;
    return false;
}


bool Graph::hasWrite(int s){
    if((adj[s].front() >= Rvertics) && (adj[s].front() < (Rvertics + Wvertics))){           // Check if there is an edge connect to a writer
        return true;
    }
    else{
        return false;
    }
}


void Graph::printGraph(){                                               // Print out the graph
    for(int i = 0; i < V; i++){
        std::cout << i << ": ";
        if(!adj[i].empty()) {
            std::list<int>::iterator it;
            for (it = adj[i].begin(); it != adj[i].end(); ++it)
                std::cout << *it << " ";
        }
        std::cout << std::endl;
    }
}


void Graph::clearEdge(int start){                           // Clear a vertice and all the edges it relates to
    for(int i = 0; i < V; i++){
        deleteEdge(i,start);
    }
    adj[start].clear();
}