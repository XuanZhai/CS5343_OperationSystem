#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include "Human.h"
#include "Graph.h"

int* TargetArray;                   // The item list
int* ReadCount;                     // The number of reader for each item
std::mutex* rMutex;
std::mutex* rwMutex;
std::mutex GraphMutex;               // Graph mutex ensures only one thread can change the graph at a time
std::mutex PrintMutex;               // Print mutex ensures only one thread can print the out at a time
int toStart = 0;                    // Start signal
int Arraysize = 0;                  // The size of Target Array
std::vector<Human> Writerlist;          // A list of reader
std::vector<Human> Readerlist;          // A list of writer
Graph* g;                           // The graph for single instance avoidance

int* IsBlock;                       // Which item is blocked
bool Dobonus = false;               // Check if the program will do the extra credit task



unsigned short xsub1[3];
void mySleep(){                     // Function for sleep
    int s, ms;
    struct timespec t0, t1;
    s = jrand48(xsub1) % 3;
    ms = jrand48(xsub1)  % 100;
    if (s == 0 && ms < 50)
        ms += 50;
    t0.tv_sec = s;
    t0.tv_nsec = ms * 1000;
    nanosleep(&t0, &t1);
}


bool Request_read(Human& nReader, int itemindex){           // Check if a reader has the permission to read
    int Writersize = Writerlist.size();
    int Readersize = Readerlist.size();

    GraphMutex.lock();                          // Lock the graph for mutual exclusion
    Graph temp = *g;                            // Deep copy the graph for future changes

    if(temp.hasWrite(itemindex+Readersize + Writersize)){           // If there is a writer who is holding the resource, predict when he release the release the resource
        temp.changeEdge((itemindex+Readersize + Writersize), nReader.GetIndex());
    }
    else{
        temp.deleteEdge(nReader.GetIndex(),(Readersize + Writersize + itemindex));    // If there is not writer, we can just change the edge's direction without waiting for the previous one to finish
        temp.addEdge((itemindex+Readersize + Writersize), (nReader.GetIndex()));
    }

    if(nReader.isholding(itemindex)){                           // If the reader who makes a request is holding that resource, no need to ask for permission
        PrintMutex.lock();
        std::cout << "Reader " << nReader.GetID() << " request to read " << itemindex << ". Granted" << std::endl;
        PrintMutex.unlock();
        GraphMutex.unlock();                                // Release the graph's lock
        return true;
    }
    else if(temp.isCyclic()){                               // Deny if there is a cycle
        GraphMutex.unlock();
        return false;
    }
    else if(Dobonus && IsBlock[itemindex] != -1){           // If doing the bonus work and the resource is blocked by a writer
        GraphMutex.unlock();                                // Will reject the target
        PrintMutex.lock();
        std::cout << "Reader " << nReader.GetID() << " request to read " << itemindex << ". Denied due to starving prevention" << std::endl;
        PrintMutex.unlock();
        return false;
    }
    else{
        if(g->hasWrite(itemindex+Readersize + Writersize)){
            g->changeEdge((itemindex+Readersize + Writersize), nReader.GetIndex());
        }
        else{
            g->deleteEdge(nReader.GetIndex(),(Readersize + Writersize + itemindex));
            g->addEdge((itemindex+Readersize + Writersize), (nReader.GetIndex()));                  // Implement those changes in the real graph
        }

        GraphMutex.unlock();


        rMutex[itemindex].lock();
        ReadCount[itemindex]++;
        if(ReadCount[itemindex] == 1)                   // Decide if hold the rwmutex
            rwMutex[itemindex].lock();
        rMutex[itemindex].unlock();

        nReader.hlist.push_back(itemindex);             // Add new hold.
        nReader.Incvlist(itemindex);                    // Increment its visited time

        PrintMutex.lock();
        std::cout << "Reader " << nReader.GetID() << " request to read " << itemindex << ". Granted" << std::endl;
        PrintMutex.unlock();

        return true;
    }
}


void Release_read(Human& nReader, int itemindex){           // Releasing the item
    int Writersize = Writerlist.size();
    int Readersize = Readerlist.size();
    for(int i = 0; i < nReader.hlist.size(); i++){
        if(nReader.hlist.at(i) == itemindex) {
            nReader.hlist.erase(nReader.hlist.begin() + i);             // Remove that item from the holding list
            break;
        }
    }

    GraphMutex.lock();
    g->deleteEdge((itemindex+Readersize + Writersize), (nReader.GetIndex()));   // Update the graph
    g->addEdge((nReader.GetIndex()),(Readersize + Writersize + itemindex));
    GraphMutex.unlock();


    rMutex[itemindex].lock();
    ReadCount[itemindex]--;
    if(ReadCount[itemindex] == 0)
        rwMutex[itemindex].unlock();                                // Decide if he needs to release the rwmutex
    rMutex[itemindex].unlock();

    PrintMutex.lock();
    std::cout << "Reader " << nReader.GetID() << " released " << itemindex << ".\n" << std::endl;
    PrintMutex.unlock();
}


bool Request_write(Human& nWriter, int itemindex){              // Check the permission for a writer to write
    int Writersize = Writerlist.size();
    int Readersize = Readerlist.size();

    GraphMutex.lock();                          // Lock the graph for mutual exclusion
    Graph temp = *g;

    temp.changeEdge((itemindex+Readersize + Writersize), (nWriter.GetIndex() + Readersize));            // Need to predict when the previous one releases the resource, not just change the requester's edge.

    if(nWriter.isholding(itemindex)){               // If that person is holding that resource right now, no need to request again

        PrintMutex.lock();
        std::cout << "Writer " << nWriter.GetID() << " request to write " << itemindex << ". Granted" << std::endl;
        PrintMutex.unlock();

        GraphMutex.unlock();
        return true;
    }
    else if(temp.isCyclic()){                   // Deny if there is a cycle
        GraphMutex.unlock();

        PrintMutex.lock();
        std::cout << "Writer " << nWriter.GetID() << " request to write " << itemindex << ". Denied.\n" << std::endl;
        PrintMutex.unlock();
        return false;
    }
    else if(Dobonus && IsBlock[itemindex] != -1){       // Reject if it got blocked
        GraphMutex.unlock();

        PrintMutex.lock();
        std::cout << "Writer " << nWriter.GetID() << " request to write " << itemindex << ". Denied.\n" << std::endl;
        PrintMutex.unlock();
        return false;
    }
    else{
        g->changeEdge((itemindex+Readersize + Writersize), (nWriter.GetIndex() + Readersize));          // Implement the change in the graph

        GraphMutex.unlock();

        if(Dobonus) {                               // If it needs to do the bonus work
            while (true) {
                if (rwMutex[itemindex].try_lock()) {        // Keep trying to get the mutex lock
                    IsBlock[itemindex] = -1;                // If get it successfully, no need to set it blocked
                    break;
                }
                IsBlock[itemindex] = nWriter.GetID();       // If the mutex is currently unavailable, mark it blocked.
            }
        }
        else{
            rwMutex[itemindex].lock();                  // If no bonus work needed, just grab that mutex.
        }

        nWriter.hlist.push_back(itemindex);             // Update the holding list and the visit list
        nWriter.Incvlist(itemindex);
        PrintMutex.lock();
        std::cout << "Writer " << nWriter.GetID() << " request to write " << itemindex << ". Granted" << std::endl;
        PrintMutex.unlock();
        return true;
    }
}


void Release_write(Human& nWriter, int itemindex){              // Release the resource

    for(int i = 0; i < nWriter.hlist.size(); i++){
        if(nWriter.hlist.at(i) == itemindex) {
            nWriter.hlist.erase(nWriter.hlist.begin() + i);         // Erase the item from the holding list
            break;
        }
    }

    PrintMutex.lock();
    std::cout << "Writer " << nWriter.GetID() << " released " << itemindex << ".\n" << std::endl;
    PrintMutex.unlock();
    rwMutex[itemindex].unlock();                        // Release the resource
}


void InitializeGraph(){                         // A function to build the Graph based on all the potential request
    int Writersize = Writerlist.size();
    int Readersize = Readerlist.size();
    g = new Graph(Writersize + Readersize + Arraysize);             // Number of edges is readers count + writers count + items count
    for(int i = 0; i < Readersize; i++){
        for(int j = 0; j < Readerlist.at(i).wlist.size(); j++){                     // Add reader
            g->addEdge(i,(Writersize+Readersize+Readerlist.at(i).wlist.at(j)));             // Readers' location is i
        }
    }

    for(int i = 0; i < Writersize; i++){
        for(int j = 0; j < Writerlist.at(i).wlist.size(); j++){                     // Add writer
            g->addEdge((i+Readersize) , (Writersize+Readersize+Writerlist.at(i).wlist.at(j)));      // Writer's location is readersize + i, i,e. if reader size is 2, writer 2's index is 3
        }
   }
    g->setGraph(Readersize,Writersize);
}


void* RThread(void* nReaderindex){
    int loc = *(int*)nReaderindex;          // Reader's index
    while (true){
        if(toStart == 1)                    // Wait for the start signal
            break;
    }

    while (true){
        int selected = nrand48(xsub1) % Readerlist.at(loc).wlist.size();            // Randomly choose a potential request
        int current = Readerlist.at(loc).wlist.at(selected);
        bool request = Request_read(Readerlist.at(loc),current);            // Doing request read

        if(request){
            Readerlist.at(loc).vlist.at(selected)++;                            // Update visited time

            PrintMutex.lock();                                                  // Avoid two threads doing output at the same time.
            std::cout << "Reader " << Readerlist.at(loc).GetID() << ": the value of item " << current << " is " << TargetArray[current] << ".\n" << std::endl;
            PrintMutex.unlock();

            mySleep();

            int islease = nrand48(xsub1)%3;                                     // Check if the thread wants to release the thread
            if(islease == 0){
                Release_read(Readerlist.at(loc),current);
            }

        }

        if(Readerlist.at(loc).AllVisited()){                                // Check if every item that the reader needed has been read at least once
            int leave = nrand48(xsub1)%5;
            if(leave == 0){
                for(int i = 0; i < Readerlist.at(loc).hlist.size(); i++) {      // Post processing if decide to leave
                    rMutex[Readerlist.at(loc).hlist.at(i)].lock();          // Release all the Mutex if he is the only one left.
                    ReadCount[Readerlist.at(loc).hlist.at(i)]--;
                    if(ReadCount[Readerlist.at(loc).hlist.at(i)] == 0)
                        rwMutex[Readerlist.at(loc).hlist.at(i)].unlock();
                    rMutex[Readerlist.at(loc).hlist.at(i)].unlock();
                }
                Readerlist.at(loc).hlist.clear();                           // Clear everything he held
                GraphMutex.lock();
                g->clearEdge(Readerlist.at(loc).GetIndex());                // Update his leaving on the graph.
                GraphMutex.unlock();
                break;
            }
        }
    }
    mySleep();

    PrintMutex.lock();
    std::cout << "Reader " << Readerlist.at(loc).GetID() << " finished.\n" << std::endl;
    PrintMutex.unlock();
    pthread_exit(0);
}


void* WThread(void* nWriterindex){
    int loc = *(int*)nWriterindex;              // Writer's index
    while (true){
        if(toStart == 1)                        // Wait for start
            break;
    }

    while (true){
        int selected = nrand48(xsub1) % Writerlist.at(loc).wlist.size();            // Randomly choose a potential request
        int current = Writerlist.at(loc).wlist.at(selected);
        bool request = Request_write(Writerlist.at(loc),current);               // Ask for request

        if(request){
            TargetArray[current] = Writerlist.at(loc).GetID() + (nrand48(xsub1)%11);        // Update the number if granted
            Writerlist.at(loc).vlist.at(selected)++;                                        // Update the number of visited time

            PrintMutex.lock();
            std::cout << "Writer " << Writerlist.at(loc).GetID() << ": the value of item " << current << " is updated to " << TargetArray[current] << ".\n" << std::endl;
            PrintMutex.unlock();

            mySleep();

            int islease = nrand48(xsub1)%3;                             // Check if release the mutex
            if(islease == 0){
                Release_write(Writerlist.at(loc),current);
            }
        }
        else{
            mySleep();                              // If request is denied
        }

        if(Writerlist.at(loc).AllVisited()){                // Check if it is able to end
            int leave = nrand48(xsub1)%6;
            if(leave == 0){
                for(int i = 0; i < Writerlist.at(loc).hlist.size(); i++)            // Post processing
                    rwMutex[Writerlist.at(loc).hlist.at(i)].unlock();               // Release all the resource it held
                Writerlist.at(loc).hlist.clear();
                GraphMutex.lock();
                g->clearEdge(Readerlist.size() + Writerlist.at(loc).GetIndex()); // Update the graph
                GraphMutex.unlock();
                break;
            }
        }
    }
    mySleep();

    PrintMutex.lock();
    std::cout << "Writer " << Writerlist.at(loc).GetID() << " finished.\n" << std::endl;
    PrintMutex.unlock();
    pthread_exit(0);
}



int main(int argc, char** argv) {
    srand(time(0));
    xsub1[0] = (short) (rand() % 256);
    xsub1[1] = (short) (rand() % 256);
    xsub1[2] = (short) (rand() % 256);              // Set seed for the jrand and nrand

    std::ifstream inputfile(argv[1]);           // Read the file from command line argument

    if(!inputfile) {
        std::cout << "Cannot find input file" << std::endl;
        exit(1);
    }

    int nReader,nWriter = 0;

    inputfile >> nReader >> nWriter >> Arraysize;
    if(Arraysize < 1){
        std::cout << "The array size is smaller than 1" << std::endl;
        exit(1);
    }

    for(int i = 0; i < (nReader + nWriter); i++){
        int id, k = 0;
        Human newhuman;
        inputfile >> id >> k;
        newhuman.SetID(id);                 // Create an object for each reader and writer

        for(int j = 0; j < k; j++){
            int distn = 0;
            inputfile >> distn;
            newhuman.wlist.push_back(distn);
            newhuman.vlist.push_back(0);
        }

        if(i < nReader){                    // This is reader
            newhuman.SetType(0);
            newhuman.SetIndex(i);
            Readerlist.push_back(newhuman);
        }
        else{                           // This is writer
            newhuman.SetType(1);
            newhuman.SetIndex(i-nReader);
            Writerlist.push_back(newhuman);
        }
    }
    inputfile.close();

    TargetArray = new int[Arraysize]();
    ReadCount = new int[Arraysize]();
    rMutex = new std::mutex[Arraysize];
    rwMutex = new std::mutex[Arraysize];

    if(argc == 3 && std::string(argv[2]) == "1"){           // If doing extra credit task
        Dobonus = true;
        IsBlock = new int[Arraysize];

        for(int a = 0; a < Arraysize; a++){
            IsBlock[a] = -1;
        }
    }


    InitializeGraph();                          // Build the graph with only reader/writer's potential request (dashed lines)


    pthread_t Readerthreads[Readerlist.size()];
    pthread_t Writerthreads[Writerlist.size()];
    int * param1 = new int[Readerlist.size()];
    int * param2 = new int[Writerlist.size()];

    for(int i = 0; i < Readerlist.size(); i++) {
        param1[i] = i;                                      // Reader's index, tell the thread function who he is
        pthread_create(&Readerthreads[i], NULL, RThread, &param1[i]);
    }

    for(int i = 0; i < Writerlist.size(); i++) {
        param2[i] = i;
        pthread_create(&Writerthreads[i], NULL, WThread, &param2[i]);
    }

    toStart = 1;                                            // Start the program

    for(int k = 0; k < Readerlist.size(); k++){
        pthread_join(Readerthreads[k],NULL);
    }

    for(int k = 0; k < Writerlist.size(); k++){
        pthread_join(Writerthreads[k],NULL);        // Wait for all the threads to finish
    }

    delete g;
    delete[] rwMutex;
    delete[] rMutex;
    delete[] TargetArray;
    delete[] ReadCount;
    delete[] param1;
    delete[] param2;                                        // Post processing

    if(argc == 3 && std::string(argv[2]) == "1"){           // If doing extra credit task
        delete[] IsBlock;
    }


    std::cout << "\n\n============= Program finished, see your next time! ================== \n" << std::endl;

    return 0;
}
