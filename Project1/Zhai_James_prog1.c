#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
//, I’ve found out that rand() is not re-entrant and thread-safe,
// and I’ve also found out that “The srand function sets the starting point for generating a series of pseudorandom integers in the current thread.”
// https://docs.microsoft.com/en-us/previous-versions/f0d4wb4t(v=vs.140)?redirectedfrom=MSDN
// So in below I've also setted the srand for each thread

int* NuminThread;                   // Global variables
int* ThreadResult;
int ready = 0;

void* Threads(void* param){
    int loc = *(int*)param;
    srand(time(0)*(loc+100000));              // Initialize the random number generator
    while (1){
        if(NuminThread[loc] == -1 && ready == 1) {
            int newnumber = rand() % 100;
            printf("thread %d generated %d\n", loc, newnumber);
            NuminThread[loc] = newnumber;
        }
        else if(ready < -1){                            // When a final winner is determined
            int winner = 0 - ready - 2;                 // redo the calculation to find who is the winner
            if(ThreadResult[loc] == winner)
                printf("Thread %d win\n",loc);
            else
                printf("Thread %d lose\n",loc);
            break;
        }
    }
    pthread_exit(0);            // end the thread
}


void CheckWinner(int nThread,int nRound){           // A function that will check who are the winners in each round and update their scores.
    int max = NuminThread[0], maxloc = 0;
    for(int i = 0; i < nThread; i++){
        if(NuminThread[i] > max){
            max = NuminThread[i];
            maxloc = i;
        }
    }
    printf("\n=============================\n");
    for(int i = 0; i < nThread; i++){           // If multiple threads hold the same highest number
        if(NuminThread[i] == max){
            ThreadResult[i]++;
            printf("Thread %d wins round %d \n", i,nRound);
        }
    }
    printf("=============================\n\n");
}


int main(int argc, char** argv) {
    if(argc != 3){
        printf("Error about command line argumeny\n");
        exit(1);
    }

    srand(time(NULL)*100000);              // Initialize the random number generator
    int nThread = atoi(argv[1]);
    int nRound = atoi(argv[2]);

    NuminThread = (int*)calloc(nThread,sizeof(int));        // Allocate space for the global variables
    ThreadResult = (int*)calloc(nThread,sizeof(int));
    int* param = (int*)calloc(nThread,sizeof(int));

    pthread_t tid[nThread];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    for(int a = 0; a < nThread; a++){
        param[a] = a;
        NuminThread[a] = -1;                            // Initialize/empty the number board to -1 since a random number can be 0, so avoid confusion.
        pthread_create(&tid[a],&attr,Threads,&param[a]);     // Create all the threads
    }

    printf("\nThe program starts!\n\n");
    for(int i = 0; i < nRound; i++){
        ready = 1;                      // Inform the threads to send in its number
        while (1){
            int total = 0;
            for(int j = 0; j < nThread; j++){
                if(NuminThread[j] != -1)
                    total++;
            }
            if(total == nThread)
                break;              // Wait for all the threads to send in the number and break
        }
        ready = 0;                  // Stop the current round
        CheckWinner(nThread,i);       // Check who wins in this round

        for(int j = 0; j < nThread; j++)
            NuminThread[j] = -1;            // Clear the number board
    }

    int fwinner = ThreadResult[0], maxloc = 0;
    for(int c = 0; c < nThread; c++){                   // Find the final winner
        printf("In the end thread %d gets %d points\n",c, ThreadResult[c]);
        if(ThreadResult[c] > fwinner){
            fwinner = ThreadResult[c];
            maxloc = c;
        }
    }

    printf("\n===========RESULT===========\n");
    ready = 0 - fwinner - 2;         // ready will become a negative number which can pass the winner's thread number to each thread. e.g. if thread 0 won, ready will be -2


    for(int k = 0; k < nThread; k++){
        pthread_join(tid[k],NULL);
    }
    free(NuminThread);
    free(ThreadResult);
    free(param);
    printf("\nThe program is finished. See you!\n");
    return 0;
}
