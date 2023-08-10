//Yasincan Bozkurt
//2304202
// standard library headers
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include <signal.h>

#define memory 4096
// Declaration of global variables
ucontext_t Main;
int remainingThreads = 5;
// ThreadInfo struct definition
typedef struct ThreadInfo {
    ucontext_t context;
    int state;
    int cpu_count;
    int io_count;
    int cpu_burst_time;
    int io_burst_time;
    int statechange;
    int index;
} ThreadInfo;
// Pointer arrays to hold ThreadInfo structs
struct ThreadInfo** Threads;
struct ThreadInfo** ThreadsArray;
// Defining the size of the Thread array
int size = 6;
// Arrays to hold CPU burst times and I/O burst times for each of 3 steps
int cpu_burst_times0[5], io_burst_times0[5];
int cpu_burst_times1[5], io_burst_times1[5];
int cpu_burst_times2[5], io_burst_times2[5];


void io();
void runThread(int index);
void exitThread(int index);

// This function displays the cpu count of a given thread.
void display_function(int i) {
    char m[5 * (i + 1)];
    memset(m, ' ', sizeof(m) - 1); // Initialize all elements to ' ' except the last one.
    m[sizeof(m) - 1] = '\0'; // Ensure null termination.
    printf("%s T%d: %d\n", m, i, ThreadsArray[i]->cpu_count);
}

// This function counts the cpu and I/O bursts for a given thread.
void count(int i) {
    if (ThreadsArray[i]->state != 2) return;

    printf("running>T%d ready>", i);
    for (int m = 1; m < size; m++) {
        if (ThreadsArray[m]->state == 1) {
            printf("T%d ", m);
        }
    }
    printf("finished>");
    for (int m = 1; m < size; m++) {
        if (ThreadsArray[m]->state == 4) {
            printf("T%d ", m);
        }
    }
    printf(" ");
    io();

    int currentState = ThreadsArray[i]->state;
    int currentCPUCount = ThreadsArray[i]->cpu_count;
    int cpuBurstTime = ThreadsArray[i]->cpu_burst_time;
    if (currentCPUCount == cpuBurstTime - 2) {
        currentState = 3; // I/O state.
        ThreadsArray[i]->io_count = 0; // Reset the IO counter
        ThreadsArray[i]->statechange++; // CPU to IO.
    } else {
        currentState = 1; // Ready state.
    }
    
    ThreadsArray[i]->cpu_count = currentCPUCount + 1;
    display_function(i);
    ThreadsArray[i]->cpu_count++;
    display_function(i);
    ThreadsArray[i]->state = currentState;
}

// This function initializes the threads, both in Threads and ThreadsArray.
void initializeThread() {
    // Allocating memory for Thread structures
    Threads = (ThreadInfo**)malloc(sizeof(ThreadInfo*) * size);
    ThreadsArray = (ThreadInfo**)malloc(sizeof(ThreadInfo*) * size);

    // Loop to initialize each thread
    for (int index = 0; index < size; index++) {
        // Allocate memory for each ThreadInfo object in Threads
        ThreadInfo *newThread = (ThreadInfo*)malloc(sizeof(ThreadInfo));
        newThread->state = 0;
        Threads[index] = newThread;
        
        // Allocate memory for each ThreadInfo object in ThreadsArray and set to NULL
        ThreadInfo *arrayThread = (ThreadInfo*)malloc(sizeof(ThreadInfo));
        arrayThread = NULL;
        ThreadsArray[index] = arrayThread;
    }
}

int createThread() {
    for (int i = 1; i < size; i++) {
        if (ThreadsArray[i] == NULL) {
            for (int j = 1; j < size; j++) {
                if (Threads[j]->state == 0) {
                    Threads[j]->cpu_burst_time = cpu_burst_times0[i - 1]; // first step cpu burst time
                    Threads[j]->io_burst_time = io_burst_times0[i - 1];
                    Threads[j]->index = i;
                    Threads[j]->state = 1;
                    ThreadsArray[i] = Threads[j];
                    getcontext(&Threads[j]->context);
                    Threads[j]->context.uc_link = &Threads[j]->context;
                    Threads[j]->context.uc_stack.ss_sp = malloc(memory);
                    Threads[j]->context.uc_stack.ss_size = memory;
                    makecontext(&Threads[j]->context, (void*)runThread, 1, j);
                    return 1;
                }
            }
        }
    }
    return -1;
}

void io() {
    puts("IO>");
    for (int k = 1; k < size; ++k) {
        if (ThreadsArray[k]->state != 3) {
            continue;
        }
        printf("T%d ", k);
        ThreadsArray[k]->io_count += 2;
        
        if (ThreadsArray[k]->io_count != ThreadsArray[k]->io_burst_time) {
            continue;
        }
        
        if (ThreadsArray[k]->statechange == 5) {
            ThreadsArray[k]->state = 4;
            --remainingThreads;
            continue;
        }

        ThreadsArray[k]->state = 1;
        ThreadsArray[k]->statechange++;
        ThreadsArray[k]->cpu_count = 0;
        
        if (ThreadsArray[k]->statechange == 2) {
            ThreadsArray[k]->cpu_burst_time = cpu_burst_times1[k - 1];
            ThreadsArray[k]->io_burst_time = io_burst_times1[k - 1];
        } else if (ThreadsArray[k]->statechange == 4) {
            ThreadsArray[k]->cpu_burst_time = cpu_burst_times2[k - 1];
            ThreadsArray[k]->io_burst_time = io_burst_times2[k - 1];
        }
    }
    puts("\n");
}


void runThread(int index) {
    alarm(2);
    if (ThreadsArray[index]->state == 1) {
        ThreadsArray[index]->state = 2;
        count(index);
    }
    sleep(2);
    swapcontext(&Threads[index]->context, &Threads[0]->context);
}

void exitThread(int index) {
    free(ThreadsArray[index]->context.uc_stack.ss_sp);
    if (remainingThreads == 0) {
        for (int i = 1; i < size; i++) {
            ThreadsArray[index]->state = 0;
        }
    }
}
// This function handles the SRTF (Shortest Remaining Time First) scheduling.
void schedular() {
    int runIndex = 0;
    int num = 10000;
    for (int i = 1; i < size; i++) {
        if (ThreadsArray[i]->state == 1) {
            if ((ThreadsArray[i]->cpu_burst_time - ThreadsArray[i]->cpu_count) < num) {
                num = ThreadsArray[i]->cpu_burst_time - ThreadsArray[i]->cpu_count;
                runIndex = i;
            }
        }
    }
    if (runIndex == 0) {
        if (remainingThreads == 0) {
            printf("running> ready> finished>T1 T2 T3 T4 T5  IO>");
        }
        else {
            printf("running> ready> finished> ");
            for (int i = 1; i < size; i++) {
                if (ThreadsArray[i]->state == 4) {
                    printf("T%d ", i);
                }
            }
            io();
            if (remainingThreads == 0) {
                printf("running> ready> finished>T1 T2 T3 T4 T5  IO>");
            }
            else {
                alarm(2);
            }
            sleep(2);
            printf("\n");
        }
    }
    else {
        swapcontext(&Threads[0]->context, &Threads[runIndex]->context);
    }
}
// Main function
int main() {
  
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

srand(time(NULL));




    signal(SIGALRM, schedular);
    initializeThread();
    for (int i = 1; i < 6; i++) {
        int error = createThread();
    }
    getcontext(&Threads[0]->context);
    Threads[0]->context.uc_link = &Threads[0]->context;
    Threads[0]->context.uc_stack.ss_sp = malloc(memory);
    Threads[0]->context.uc_stack.ss_size = memory;
    schedular();
    while (1) {
    }
    return 0;
}
//Yasincan Bozkurt
//2304202