#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


// The code passed the prog2test-1.txt under Windows X64 with WSL2.
// The program does accept the extra bonus with the additional thread; if it does not show on the output, you'll probably need to modify the delay.



typedef struct Customer{
    int id;
    int totalmoney;
    int nitem;          // Since itemlist uses dynamic memory allocation, it cannot use sizeof to get the number of items.
    int* itemlist;      // A list of interested items

} Customer;

struct Customer* create_new(int nid,int ntotalmoney,int nitem, int* nitemlist){     // A constructor function of Customer
    Customer* newCustomer = malloc(sizeof(int)*(3+nitem));      // Size is the size of nid+ntotalmoney+nitem+nitemlist
    newCustomer->id = nid;
    newCustomer->totalmoney = ntotalmoney;
    newCustomer->nitem = nitem;

    newCustomer->itemlist = (int*)calloc(nitem, sizeof(int));
    memcpy(newCustomer->itemlist, nitemlist, nitem* sizeof(int));       // Deep copy of the item list
    return newCustomer;
}

void freeCustomer(struct Customer* c){              // a destructor of Customer
    free(c->itemlist);
    free(c);
}

// Reference: https://www.nku.edu/~foxr/CSC362/CODE/queue.c
typedef struct node				// a node in a linked list
{
    struct Customer* item;					// consists of the int item
    struct node *next;			// and a pointer to the next item in the linked list
} node;

typedef struct Queue			// the Queue will have a pointer to the front
{								// and a pointer to the rear of the queue
    struct node *front;			// the pointers will equal NULL if the list is empty
    struct node *rear;			// or will point at the same location for a 1-item queue
} Queue;

int empty(struct Queue *q)			// see if the queue is empty, if so, return 0, else
{									// else return 1
    if(q->front==NULL) return 0;
    else return 1;
}

void enqueue(struct Queue *q, struct Customer* i)	// this function will create a new node storing i and
{										// add it to the rear of the queue
    struct node *temp;
    temp = (struct node *) malloc(sizeof(struct node));
    temp->item = i;
    temp->next = NULL;
    if(empty(q)!=0)						// if the queue is not empty, add it at the rear
    {									// adjusting the last node's next pointer to point
        q->rear->next = temp;			// at it
    }
    else
        q->front = temp;				// otherwise it is the only item, adjust front to point at it
    q->rear = temp;						// in either case point rear at the newly added item

}

struct Customer* dequeue(struct Queue *q)			// dequeue the next item and return the int value,
{										// deallocating the memory for node
    //int* temp0 = (int*)malloc(sizeof(int));
    struct Customer* temp;
    struct node *temp2;					// temp2 will point at the dequeued node temporarily
    if(empty(q)!=0)						// if there are items in the queue, we can dequeue the first
    {
        temp = create_new(q->front->item->id,q->front->item->totalmoney,q->front->item->nitem, q->front->item->itemlist);
        temp2 = q->front;				// temp2 points at the node
        q->front = q->front->next;		// reset front to point at the next item in the queue
        free(temp2->item->itemlist);
        free(temp2->item);
        free(temp2);					// and deallocate the front node
        if(q->front==NULL) q->rear=NULL; // if the queue is now empty, set rear to NULL also
    }
    else printf("ERROR, Queue empty, cannot dequeue\n");	// if queue was empty, output error message
    return temp;						// return dequeued item or -999 if error
}


void print_queue(struct Queue *q) {         // Print out the item in the queue (in this project it's the Customer id)
    struct node *current = q->front;

    while (current != NULL) {
        printf("%d\n", current->item->id);
        current = current->next;
    }
}


void clear_queue(struct Queue *q){           // Clear the queue and free the memory
    struct node *current = q->front;

    while (current != NULL){
        struct Customer* temp = dequeue(q);
        free(temp->itemlist);
        free(temp);
        current = q->front;
    }
    free(q);
}


//Global Variable
struct Queue *q;
int* TableStatus;           // Which step of auction each table is in, use for synchronization.
int* TableCustomer;         // For each table, if there's a customer, it shows the customer's id, else empty table shows -1.
int auctionflag = -1;       // FlAG for running the auction.
int* BidBoard;              // A board collects bidders' money.
int currentItem;            // Current item type
int highestbid = 0;         // Record highest bidding for each round
int highesttable = 0;       // Which table/customer puts the highest money.

int totalauctioned = 0;
int totalnonauctioned = 0;
int totalmoney = 0;         // Record final summary.
int extracustomer = 0;      // Trigger for adding a new customer
pthread_mutex_t queuelock;      // A clock manage the queue.

int* itemlist(int ntype, int number, int tflag) {       // Create the item list
    int *list = (int *) calloc(number, sizeof(int));
    if(tflag == 1){
        int start = 1;
        for(int i = 0; i < number; i++){
            list[i] = start;
            if(start == ntype)
                start = 1;          // If reach the maximum number of type, go back to the first
            else
                start++;            // 1, 2, 3 ....
        }
    }
    else{                           // Else the items is to be generated randomly
        for(int i = 0; i < number; i++)
            list[i] = 1 + rand() % ntype;
    }
    printf("\n");
    return list;
}


int* SetList(int* list, int size, int target){      //  Set the list, used to reset every int array (to 0 or -1)
    for(int i = 0; i < size; i++){
        list[i] = target;
    }
    return list;
}


int MakeBid(int item, int* interestlist, int size, int minimoney, int maxmoney){    // Make a bid for an item
    if(maxmoney == 0)
        return 0;
    for(int i = 0; i < size; i++){
        if(interestlist[i] == item)
            return (minimoney+rand()%(maxmoney-minimoney + 1));     // bid is a random number between mini and max.
    }
    return 0;
}


void CheckUpdate(int* list, int size, int target, int require){     // keep spinning until every table updated its info.
    while (1){                                                  // size is the size of the list, target is the step
        int total = 0;
        for(int j = 0; j < size; j++){
            if(list[j] == target)
                total++;
        }
        if(total == require)
            break;
    }
}


void* ExtraThreads(void* param){                    // It's for the extra credit, extra work for the thread.
    srand(time(0)*(100000));              // Initialize the random number generator
    while (1) {
        int Delay = 100000;
        int Dtry = 1;
        int x;
        for (int i = 0; i < Dtry; i++) {
            for (int j = 0; j < Delay; j++)         // Running spin
                x = 1;
        }
        int randomnumber = rand() % 5;
        if (randomnumber == 4) {
            int* newlist = (int*)calloc(1,sizeof(int));
            newlist[0] = 1;
            Customer *newCustomer = create_new(100+rand()%100,100+rand()%100,1,newlist);        // Create a new customer, it can be modified.
            free(newlist);
            extracustomer = 1;
            while (1){
                if(extracustomer == 2){             // Waiting for the beginning of a new round
                    pthread_mutex_lock(&queuelock);
                    enqueue(q, newCustomer);            // Add it to the queue, it needs a mutex lock to avoid customer-producer problem.
                    printf("\nCustomer %d added to the queue.\n",newCustomer->id);
                    pthread_mutex_unlock(&queuelock);
                    extracustomer = 0;
                    break;
                }
                else if(auctionflag == 5)           // Waiting until program end.
                    break;
            }
        } else {
            if (Dtry >= 2)
                Dtry = Dtry / 2;
            else
                Dtry = 1;
        }
        if(auctionflag == 5)                // Exit
            break;
    }
    pthread_exit(0);            // End the thread
}


void* Threads(void* param){
    int loc = *(int*)param;
    struct Customer* currentCustomer = NULL;
    srand(time(0)*(loc+100000));              // Initialize the random number generator


    while (1) {
        if (TableStatus[loc] == 0 && auctionflag == -1) {
            pthread_mutex_lock(&queuelock);
            if (TableCustomer[loc] == -1 && empty(q) == 1) {        // The critical section should include every action that relates to access to the queue.
                currentCustomer = dequeue(q);                   // Get a customer from the queue to the table, it needs a mutex lock to avoid two tables want the same customer.
                TableCustomer[loc] = currentCustomer->id;
                printf("Customer %d assigned to table %d\n", currentCustomer->id, loc);
            }
            pthread_mutex_unlock(&queuelock);
            TableStatus[loc] = 1;
        }
        if(TableStatus[loc] == 1 && auctionflag == -1 && TableCustomer[loc] != -1) {        // If a table has a customer
            while (1) {
                if (auctionflag == 1 && TableStatus[loc] == 1) {
                    BidBoard[loc] = MakeBid(currentItem, currentCustomer->itemlist, currentCustomer->nitem, 1, currentCustomer->totalmoney);  // Make a bid
                    printf("Customer %d at table %d bids %d.\n", currentCustomer->id, loc, BidBoard[loc]);
                    TableStatus[loc] = 2;
                }
                if (auctionflag == 2  && TableStatus[loc] == 2) {           // Make the second bid.
                    int mainorinc = rand() % 2;
                    if (mainorinc == 1) {             // 50% percent chance for placing a higher bid.
                        BidBoard[loc] = MakeBid(currentItem, currentCustomer->itemlist, currentCustomer->nitem,
                                                BidBoard[loc], currentCustomer->totalmoney);  // Make a bid (minimal number is the currect money, maximum number is the total money)
                    }
                    printf("Customer %d at table %d bids %d.\n", currentCustomer->id, loc, BidBoard[loc]);
                    TableStatus[loc] = 3;
                }
                if (auctionflag == 3 && (TableStatus[loc] == 3 || TableStatus[loc] == 2)) {     // Check if the table's customer won the bid.
                    if (highesttable == loc) {
                        printf("\nCustomer %d won the auction, with amount %d.\n", currentCustomer->id, highestbid);
                        currentCustomer->totalmoney = currentCustomer->totalmoney - highestbid;
                        int *newlist = (int *) calloc(currentCustomer->nitem - 1, sizeof(int));
                        int iserased = 0, iter = 0;
                        for (int a = 0; a < currentCustomer->nitem; a++) {
                            if (currentCustomer->itemlist[a] == currentItem && iserased == 0) {
                                iserased = 1;
                            } else {
                                newlist[iter] = currentCustomer->itemlist[a];
                                iter++;
                            }
                        }

                        free(currentCustomer->itemlist);
                        currentCustomer->itemlist = newlist;
                        currentCustomer->nitem = currentCustomer->nitem - 1; // Processing checkout, cost money, remove item from the interested list.

                        totalmoney = totalmoney + highestbid;
                        totalauctioned++;

                    }
                    TableStatus[loc] = 4;
                }

                if ((currentCustomer->totalmoney != 0 || currentCustomer->nitem != 0) && (TableStatus[loc]  != 1) && auctionflag == 4) {    // Check if the customer will leave.
                    int isleave = (1 + rand() % 1000);
                    if (isleave > 900) {                 // If the customer still has money and interested item, he has 10% chance will also leave the table and go back to the queue.
                        TableCustomer[loc] = -1;
                        printf("%d leave the table. %d money left. %d interested items left\n", currentCustomer->id,currentCustomer->totalmoney, currentCustomer->nitem);
                        pthread_mutex_lock(&queuelock);
                        enqueue(q, currentCustomer);        // Need mutex lock to add the customer back to the queue.
                        pthread_mutex_unlock(&queuelock);
                        TableStatus[loc] = 0;
                        break;
                    }
                    TableStatus[loc] = 1;
                }
                else if ((currentCustomer->totalmoney == 0 || currentCustomer->nitem == 0) && (TableStatus[loc] != 1) && auctionflag == 4) {    // If no money or interested item, the customer will have to leave.
                    printf("%d leave the table. %d money left. %d interested items left\n", currentCustomer->id,currentCustomer->totalmoney, currentCustomer->nitem);
                    TableCustomer[loc] = -1;
                    freeCustomer(currentCustomer);
                    TableStatus[loc] = 0;
                    break;
                }
                if(auctionflag == 5)
                    break;
            }
        }
        if (auctionflag == 5) {                 // End the auction.
            if (TableCustomer[loc] != -1) {
                TableCustomer[loc] = -1;
                freeCustomer(currentCustomer);
            }
            break;
        }
    }
    pthread_exit(0);            // End the thread
}



int main() {
    q = (struct Queue *) malloc(sizeof(struct Queue));	// allocate the queue
    q->front = NULL;			                // initialize the queue's pointers to NULL
    q->rear = NULL;

    int Tablenum, Typenum, Customernum, Itemnum,createType,Extracredit;

    FILE* inputfile = fopen("prog2test-1.txt", "r");
    fscanf(inputfile,"%d %d %d %d %d %d", &Tablenum, &Typenum, &Customernum, &Itemnum,&createType,&Extracredit);
    srand(time(NULL)*100000);              // Initialize the random number generator


    for(int i = 0; i < Customernum; i++){
        int id,totalmoney,nitem, tempitem;
        fscanf(inputfile, "%d %d %d", &id,&totalmoney,&nitem);
        int* itemlist = (int*)calloc(nitem, sizeof(int));
        for(int j = 0; j < nitem; j++){
            fscanf(inputfile,"%d",&tempitem);
            itemlist[j] = tempitem;
        }
        struct Customer* newCustomer = create_new(id,totalmoney,nitem,itemlist);
        free(itemlist);
        enqueue(q,newCustomer);                                     // Add initial customers to the queue.
        printf("Customer %d added to the queue.\n",newCustomer->id);
    }

    int* ItemList = itemlist(Typenum,Itemnum,createType);
    TableStatus = (int*)calloc(Tablenum,sizeof(int));
    TableCustomer = (int*)calloc(Tablenum,sizeof(int));
    for(int i = 0; i < Tablenum; i++){
        TableCustomer[i] = -1;                      // Make all the tables empty.
    }


    pthread_t tid[Tablenum];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int* param = (int*)calloc(Tablenum,sizeof(int));
    pthread_mutex_init(&queuelock, NULL);


    for(int a = 0; a < Tablenum; a++){
        param[a] = a;
        pthread_create(&tid[a],&attr,Threads,&param[a]);     // Create all the threads for the tables.
    }

    pthread_t extid;
    if(Extracredit == 1) {
        pthread_create(&extid, &attr, ExtraThreads, NULL);   // Create the thread of the extra work.
    }

    for(int i = 0; i < Itemnum; i++){                       // For each item
        auctionflag = -1;

        CheckUpdate(TableStatus,Tablenum,1,Tablenum);   // Checking for table's availability

        if(extracustomer == 1){                     // If the extra thread wants to add a new customer to the queue
            extracustomer = 2;
            while (1){
                if(extracustomer == 0)              // Wait for the thread to add the new customer
                    break;
            }
        }

        int currncustomer = 0;
        printf("\n============================ Round %d ============================\n\n", i);
        printf("The following customers are eligible for this round of auction : ");
        for(int j = 0; j < Tablenum; j++){
            if(TableCustomer[j] != -1) {        // If a table has a customer, TableCustomer will show his id, if no customer the number will be -1.
                printf("%d ", TableCustomer[j]);
                currncustomer++;
            }
        }

        currentItem = ItemList[i];
        printf("\n\nItem of type %d up for auction\n", currentItem);

        BidBoard = (int*)calloc(Tablenum,sizeof(int));
        SetList(BidBoard,Tablenum,-1);
        highestbid = 0;
        highesttable = 0;
        printf("--------------------- Start Bidng ---------------------\n");
        auctionflag = 1;                                                // Start Biding

        CheckUpdate(TableStatus,Tablenum,2,currncustomer);          // Wait for first round of biding finish.

        int nextstep = 0;
        for(int j = 0; j < Tablenum; j++){
            if(BidBoard[j] > highestbid) {
                highestbid = BidBoard[j];           // Find a bidder with a higher biding price.
                highesttable = j;
                nextstep = 1;
            }
            else if(BidBoard[j] == highestbid && highestbid != 0)
                nextstep = 2;               // If there is a same highest bid
        }

        if(nextstep == 1){                   // If there is only one bidding winner
            auctionflag = 3;                // Processing checkout for the auction.
        }
        else if(nextstep == 2){
            printf("---------------- Second round of bidding needed ----------------\n");
            auctionflag = 2, nextstep = 0;                // Start Round 2

            CheckUpdate(TableStatus,Tablenum,3,currncustomer);      // Wait for the second round of bid finish.

            for(int j = 0; j < Tablenum; j++){
                if(BidBoard[j] > highestbid) {
                    highestbid = BidBoard[j];           // Find a bidder.
                    highesttable = j;
                    nextstep = 1;
                }
                else if(BidBoard[j] == highestbid && highestbid != 0)
                    nextstep = 2;               // If there is a same highest bid
            }

            if(nextstep == 1){
                auctionflag = 3;                // Find a winner. Processing checkout for the auction.
            }
            else{
                printf("\nThere is a tie, so no one win the auction\n\n");
                totalnonauctioned++;
                auctionflag = 4;
            }

        }
        else{
            printf("\nNo one won the auction\n\n");                     // Discard the item since no one interested it
            totalnonauctioned++;
            auctionflag = 4;
        }

        if(auctionflag == 3){                                           // Wait for the checkout finished.
            CheckUpdate(TableStatus,Tablenum,4,currncustomer);
            auctionflag = 4;
        }

        if(auctionflag == 4){
            while(1){                                   // Wait for the leaving finish.
                int total = 0;
                for(int j = 0; j < Tablenum; j++){
                    if(TableStatus[j] == 0 ||TableStatus[j] == 1 || TableStatus[j] == 3)
                        total++;
                }
                if(total == Tablenum)
                    break;
            }
        }
        free(BidBoard);
        printf("\n============================ Round Finished ============================\n\n");
    }
    auctionflag = 5;

    printf("\n\nAuction Summary\n");
    printf("Total number of item auctioned: %d\n", totalnonauctioned+totalauctioned);
    printf("Total number of item successfully auctioned %d\n", totalauctioned);
    printf("Total money spent in the auction %d\n", totalmoney);
    printf("Thank you! See you next time.\n");


    for(int k = 0; k < Tablenum; k++){
        pthread_join(tid[k],NULL);
    }
    if(Extracredit == 1) {
        pthread_join(extid, NULL);
    }
    clear_queue(q);
    free(param);
    free(TableCustomer);
    free(TableStatus);
    free(ItemList);
    pthread_mutex_destroy(&queuelock);
    return 0;
}
