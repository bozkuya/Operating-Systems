//Yasincan Bozkurt
//2304202
// Include required standard libraries
#include <stdio.h>
#include <time.h>
#include <ucontext.h>
#include <signal.h>
#include <stdlib.h>

#define memory 4096 // Define memory size
int remainingThreads;
 remainingThreads = 5;
// Define a structure to hold thread information
typedef struct thread_info{
    ucontext_t context;
    int state; // 0 for empty, 1 for ready, 2 for running, 3 for I/O, 4 for finished.
    int cpu_count; // It increases +2 with every CPU counter value.
    int io_count; // It increases +2 with every I/O operation.
    int cpu_burst_time; // CPU Burst time (changes at each step)
    int io_burst_time;   
    int change_state; // Odd value -> from CPU to I/O, even value -> from I/O to CPU
    int index;
} thread_info;
struct thread_info **threads;
struct thread_info **arr;
int size = 6; // The size of the arr
int start = 1; // Flag to determine initial lottery values based on CPU Burst Time
int cpu_burst_times0[5]; 
int io_burst_times0[5]; // Holds the CPU burst and IO times for step 1
int cpu_burst_times1[5]; 
int io_burst_times1[5]; // Holds the CPU burst and IO times for step 2
int cpu_burst_times2[5]; 
int io_burst_times2[5]; // Holds the CPU burst and IO times for step 3
int total_burst_time[5]; // Holds the total CPU burst time of the threads
int total_burst_time_allT; // Holds the sum of the total CPU burst times of the threads

// Functions

void io();
void runThread(int index);
void exitThread(int index);

void displayFunc(int i){
    char m[5*(i+1)];
    for(int j = 0; j < 5*i+4; j++){
        m[j] = ' ';
    }
    m[5*(i) + 4] = '\0';
    printf("%s T%d: %d\n",m,i,arr[i]->cpu_count);
}

void counter(int i) {
    // Check the state of the thread
    if (arr[i]->state == 2) { // Running state
        printf("Running > T%d Ready > ", i);
        for (int m = 1; m < size; m++) {
            if (m != i && arr[m]->state == 1) { // Ready state
                printf("T%d ", m);
            }
        }
        printf("Finished > ");
        for (int m = 1; m < size; m++) {
            if (arr[m]->state == 4) { // Finished state
                printf("T%d ", m);
            }
        }
        printf(" ");
        io(); // Check for I/O operations

        if (arr[i]->cpu_count == arr[i]->cpu_burst_time - 2) {
            // If T[i]'s CPU operation is finished, move to I/O
            arr[i]->cpu_count++;
            displayFunc(i);
            arr[i]->cpu_count++;
            displayFunc(i);
            arr[i]->state = 3; // I/O state
            arr[i]->io_count = 0; // Reset the I/O counter
            arr[i]->change_state++; // CPU to I/O
        } else {
            // If T[i] continues CPU operation
            arr[i]->cpu_count++;
            displayFunc(i);
            arr[i]->cpu_count++;
            displayFunc(i);
            arr[i]->state = 1; // Ready state
        }
    }
}

void initializeThread(){
    // Allocate memory for the threads
    threads = malloc(sizeof(thread_info*) * size);
    arr = malloc(sizeof(thread_info*) * size);
    for (int i = 0; i < size; i++){
        threads[i] = malloc(sizeof(thread_info));
        threads[i]->state = 0; // 0 means EMPTY state, put them in arr in the createThread() function
        arr[i] = malloc(sizeof(thread_info));
        arr[i] = NULL; // Not assigned
    }
}

int createThread(){
    // arr[0] is the main thread, no need to adjust its variables
    for(int i = 1; i < size; i++){
        if(arr[i] == NULL){
            for(int j = 1; j < size; j++){
                if(threads[j]->state == 0){ // Empty state
                    threads[j]->cpu_burst_time = cpu_burst_times0[i - 1]; // First step CPU burst time
                    threads[j]->io_burst_time = io_burst_times0[i - 1]; // First step I/O burst time
                    threads[j]->index = i;
                    threads[j]->state = 1; // Ready state
                    arr[i] = threads[j];
                    getcontext(&threads[j]->context);
                    threads[j]->context.uc_link = &threads[j]->context;
                    threads[j]->context.uc_stack.ss_sp = malloc(memory);
                    threads[j]->context.uc_stack.ss_size = memory;
                    makecontext(&threads[j]->context, (void*) runThread, 1, j); // Set the function to runThread() when called
                    return 1; // ThreadArray[i] is successful
                }
            }
        }
    }
    return -1; // ThreadArray[i] is not assigned, indicates an error
}




void runThread(int index){
    alarm(2);
    // Check if the thread is in the ready state before running and going to the counter function
    if(arr[index]->state == 1){ // Ready state
        arr[index]->state = 2; // Running state
        counter(index);
    }
    sleep(2);
    swapcontext(&threads[index]->context, &threads[0]->context); // Return to the main thread
}

void exitThread(int index){
    // Free the thread's context memory (called when finished)
    free(arr[index]->context.uc_stack.ss_sp);
}

void io(){
    printf("IO>");
    for(int k = 1; k < size; k++){
        if(arr[k]->state == 3){
            // I/O operation
            printf("T%d ", k);
            arr[k]->io_count += 2;
            if(arr[k]->io_count == arr[k]->io_burst_time){
                // If this is the last I/O operation (change_state == 5), the process is finished
                if(arr[k]->change_state == 5){
                    arr[k]->state = 4; // Finished state
                    remainingThreads--;
                    // exitThread(k);
                }
                else{
                    // Thread goes to ready queue, not finished
                    arr[k]->state = 1; // Ready state
                    arr[k]->change_state++;
                    arr[k]->cpu_count = 0;
                    // Thread goes to new CPU/I/O pair, assign new burst times
                    if(arr[k]->change_state == 2){
                        arr[k]->cpu_burst_time = cpu_burst_times1[k - 1];
                        arr[k]->io_burst_time = io_burst_times1[k - 1];
                    }
                    else if(arr[k]->change_state == 4){
                        arr[k]->cpu_burst_time = cpu_burst_times2[k - 1];
                        arr[k]->io_burst_time = io_burst_times2[k - 1];
                    }
                }
            }
        }   
    }
    printf("\n");
}

void schedular(){
    // runIndex indicates the index of the next thread to be executed in the CPU
    // start variable is initially equal to 1
    int runIndex = 0;
    // Determine the weight of the threads based on their totalBurstTimes
    if(start == 1){
        total_burst_time[0] = cpu_burst_times0[0] + cpu_burst_times1[0] + cpu_burst_times2[0];
        total_burst_time[1] = cpu_burst_times0[1] + cpu_burst_times1[1] + cpu_burst_times2[1];
        total_burst_time[2] = cpu_burst_times0[2] + cpu_burst_times1[2] + cpu_burst_times2[2];
        total_burst_time[3] = cpu_burst_times0[3] + cpu_burst_times1[3] + cpu_burst_times2[3];
        total_burst_time[4] = cpu_burst_times0[4] + cpu_burst_times1[4] + cpu_burst_times2[4];
        total_burst_time_allT = total_burst_time[0] + total_burst_time[1] + total_burst_time[2] + total_burst_time[3] + total_burst_time[4];
        printf("Sharing:\n");
        printf("T1:%d/%d ",total_burst_time[0], total_burst_time_allT);
        printf("T2:%d/%d ",total_burst_time[1], total_burst_time_allT);
        printf("T3:%d/%d ",total_burst_time[2], total_burst_time_allT);
        printf("T4:%d/%d ",total_burst_time[3], total_burst_time_allT);
        printf("T5:%d/%d \n",total_burst_time[4], total_burst_time_allT);
        start = 0;
    }
    // Generate a random number between (0, totalBurstTime)
    // Based on the interval (tickets), select the next thread to execute
    // Threads with longer totalBurstTime have more tickets, meaning a larger interval
    while(remainingThreads != 0){
        int RandNum = rand() % total_burst_time_allT;
        if((0 <= RandNum) && (RandNum < total_burst_time[0])){
            if(arr[1]->state == 1){ // Ready state
                runIndex = 1;
                break;
            }
        }
        else if((total_burst_time[0] <= RandNum) && (RandNum < (total_burst_time[0] + total_burst_time[1]))){
            if(arr[2]->state == 1){ // Ready state
                runIndex = 2;
                break;
            }
        }
        else if(((total_burst_time[0] + total_burst_time[1]) <= RandNum) && (RandNum < (total_burst_time[0] + total_burst_time[1] + total_burst_time[2]))){
            if(arr[3]->state == 1){ // Ready state
                runIndex = 3;
                break;
            }
        }
        else if(((total_burst_time[0] + total_burst_time[1] + total_burst_time[2]) <= RandNum) && (RandNum < (total_burst_time_allT - total_burst_time[4]))){
            if(arr[4]->state == 1){ // Ready state
                runIndex = 4;
                break;
            }
        }
        else if(((total_burst_time_allT - total_burst_time[4]) <= RandNum) && (RandNum < total_burst_time_allT)){
            if(arr[5]->state == 1){ // Ready state
                runIndex = 5;
                break;
            }
        }
        if((arr[1]->state != 1 && arr[2]->state != 1 && arr[3]->state != 1 && arr[4]->state != 1 && arr[5]->state != 1)){
            runIndex = 0;
            break;
        }
    }
    // If runIndex is not, go to runThread and perform CPU operation
    // If runIndex is 0, it means there are no ready threads. There are two cases:
    // 1) If remainingThreads is 0, all processes are complete, no need to go to runThread(). Finish.
    // 2) If remainingThreads is not 0, there are processes in the I/O state. Handle them.
    if(runIndex == 0){
        if(remainingThreads == 0){
            printf("running> ready> finished>T1 T2 T3 T4 T5  IO>");
        }
        else{
            // I/O check. No CPU burst. Need to re-activate the alarm.
            printf("running> ready> finished> ");
            for(int i = 1; i < size; i++){
                if(arr[i]->state == 4){ // Finished state
                    printf("T%d ", i);
                }
            }
            io();
            if(remainingThreads == 0){
                printf("running> ready> finished>T1 T2 T3 T4 T5  IO>");
            }
            else{
                alarm(2);
            }
            sleep(2);
            printf("\n");
        }
    }
    else{
        swapcontext(&threads[0]->context, &threads[runIndex]->context);
    }
}
int main(){
    // Thread 0 is the main thread
    // Take input for CPU and I/O burst times
    //You can change parameters by using this part.
    cpu_burst_times0[0] = 1;
    cpu_burst_times0[1] = 2;
    cpu_burst_times0[2] = 3;
    cpu_burst_times0[3] = 4;
    cpu_burst_times0[4] = 5;
    io_burst_times0[0] = 1;
    io_burst_times0[1] = 2;
    io_burst_times0[2] = 3;
    io_burst_times0[3] = 4;
    io_burst_times0[4] = 5;

    cpu_burst_times1[0] = 1;
    cpu_burst_times1[1] = 2;
    cpu_burst_times1[2] = 3;
    cpu_burst_times1[3] = 4;
    cpu_burst_times1[4] = 5;
    io_burst_times1[0] = 1;
    io_burst_times1[1] = 2;
    io_burst_times1[2] = 3;
    io_burst_times1[3] = 4;
    io_burst_times1[4] = 5;

    cpu_burst_times2[0] = 1;
    cpu_burst_times2[1] = 2;
    cpu_burst_times2[2] = 3;
    cpu_burst_times2[3] = 4;
    cpu_burst_times2[4] = 5;
    io_burst_times2[0] = 1;
    io_burst_times2[1] = 2;
    io_burst_times2[2] = 3;
    io_burst_times2[3] = 4;
    io_burst_times2[4] = 5;

    signal(SIGALRM, schedular);
    initializeThread();
    for(int i = 1 ; i < 6; i++){
        int error = createThread();
    }
    getcontext(&threads[0]->context);
    threads[0]->context.uc_link = &threads[0]->context;
    threads[0]->context.uc_stack.ss_sp = malloc(memory);
    threads[0]->context.uc_stack.ss_size = memory;
    schedular();
    while(1){
    }
    return 0;
}
//Yasincan Bozkurt
//2304202