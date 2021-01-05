#define _GNU_SOURCE 
#include<stdio.h>
#include<pthread.h>
#include<stdint.h>
#include<unistd.h>
#include<stdlib.h>
#include<sched.h>
#include<string.h>
#include<math.h>
#include<errno.h>
#include<time.h>

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


#define THREAD_PRIORITY (99)   // RT priority for both threads
#define CPU1 (30)              // CPU for main gunction
#define CPU2 (32)              // CPU for Array thread


#define G_HZ        (1000000000)             // G_HZ = 10^9
#define SIM_IN_MINS (90)                     // Test will run for exactly these many minutes
#define MAX_ITER    (SIM_IN_MINS*60*1000*2)  // Number of iterations
              
#define ARRSIZE     (30000)                  // Array size 


/* Structure to store ticks */
typedef struct tickSt
{
    unsigned long long int tickSec;
    unsigned long long int tickNsec;

}tickSt;


/* Structure to store Array thread data */
typedef struct ArrayThDataSt
{
    tickSt tickStoreArray[MAX_ITER];

    uint32_t counter;

    struct arrSt
    {
        uint16_t A;
        uint16_t B;
        uint16_t C;

    }arrSt[ARRSIZE];

}ArrayThSt;


/* Main structure */
typedef struct tDataSt
{
    tickSt    tickStoreClk[MAX_ITER];

    uint8_t   sigVar;

    ArrayThSt thData;

}tDataSt;

tDataSt tData;


#define TASK_ASSIGN (1)
#define TASK_DONE   (2)


/* Function for array thread */
void *ArraythreadFunc()
{
    tData.thData.counter = 0;

    struct timespec t2;

    /* Busy while loop */
    while(1)
    {   
        /* Enter inside whenever main thread assign tasks */
        if(tData.sigVar == TASK_ASSIGN)
        {
            /* Perform array multiplication (kind of) */
            for(uint32_t iter=0; iter<ARRSIZE; iter++)
                tData.thData.arrSt[iter].C = (tData.thData.arrSt[iter].A + 2*iter) * (tData.thData.arrSt[iter].B + 1*iter);

            /* Set the task to done [Not used right now] */
            __sync_lock_test_and_set(&tData.sigVar, TASK_DONE);

            /* Read the current time and store it */
            clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
            tData.thData.tickStoreArray[tData.thData.counter].tickSec  = t2.tv_sec;
            tData.thData.tickStoreArray[tData.thData.counter].tickNsec = t2.tv_nsec;
            tData.thData.counter++;
        }
    }

    /* Function should never come here anyway */
    return NULL;
}




int main()
{
    /* Setting up the RT thread priority for this thread */
    struct sched_param param;
    param.sched_priority = THREAD_PRIORITY;
    int sParam;
    sParam = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    if(0 != sParam)
        handle_error_en(sParam, "pthread_setschedparam self");


    /* Binding this thread to the CPU: CPU1 */
    cpu_set_t cpuIdSelf;
    CPU_ZERO(&cpuIdSelf);
    CPU_SET(CPU1, &cpuIdSelf);

    int sAff;
    sAff = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuIdSelf);
    if (0 != sAff)
           handle_error_en(sAff, "pthread_setaffinity_np self");


    printf("Main thread binded to CPU:  %d with thread priority:    %d\n",CPU1, THREAD_PRIORITY);



    /* Creating a thread for Array multiplication */   
    pthread_t threadArray;
    pthread_create(&threadArray, NULL, ArraythreadFunc, NULL);

    /* Binding this thread to the CPU: CPU2 */
    cpu_set_t cpuId;
    CPU_ZERO(&cpuId);
    CPU_SET(CPU2, &cpuId);

    sAff = pthread_setaffinity_np(threadArray, sizeof(cpu_set_t), &cpuId);
    if (0 != sAff)
           handle_error_en(sAff, "pthread_setaffinity_np array");

    /* Setting up the RT thread priority for this thread */ 
    sParam = pthread_setschedparam(threadArray, SCHED_FIFO, &param);
    if(0 != sParam)
        handle_error_en(sParam, "pthread_setschedparam array");


    printf("Array thread binded to CPU:  %d with thread priority:    %d\n",CPU2, THREAD_PRIORITY);

    sleep(1);

    struct timespec stT, enT;
    unsigned long long int tick;

    printf("Test is ready to run now\n"); fflush(stdout);
    printf("Test will run for %d mins\n",SIM_IN_MINS); fflush(stdout);
    printf("Test will run for %d iterations\n",MAX_ITER); fflush(stdout);

    for(uint32_t iter=0; iter<MAX_ITER; iter++)
    {
        /* This set the flag for other thread to run */
        __sync_lock_test_and_set(&tData.sigVar, TASK_ASSIGN);

        /* Read the time and not it down */
        clock_gettime(CLOCK_MONOTONIC_RAW, &stT);
        tData.tickStoreClk[iter].tickSec = stT.tv_sec;
        tData.tickStoreClk[iter].tickNsec = stT.tv_nsec;

        /* Wait for 500 micro seconds to pass */
        /* This is a busy while loop */
        tick = 0;
        while(tick < 500000)
        {
            clock_gettime(CLOCK_MONOTONIC_RAW, &enT);
            tick = (enT.tv_sec - stT.tv_sec)*G_HZ + (enT.tv_nsec - stT.tv_nsec);
        }

        /* Now go back and signal the task */
    }

    printf("Experiment is completed now\n"); fflush(stdout);

    sleep(1);

    /* Writing the tick reading into files */

    FILE *fidProf1 = fopen ("totalProfTime_n00.txt","w");
    for(uint32_t tIdx=0; tIdx<MAX_ITER; tIdx++)
        fprintf(fidProf1, "%llu\n", tData.tickStoreClk[tIdx].tickSec);
    fclose(fidProf1);

    FILE *fidProf2 = fopen ("totalProfTime_n01.txt","w");
    for(uint32_t tIdx=0; tIdx<MAX_ITER; tIdx++)
        fprintf(fidProf2, "%llu\n", tData.tickStoreClk[tIdx].tickNsec);
    fclose(fidProf2);

    FILE *fidProf3 = fopen ("totalProfTime_n10.txt","w");
    for(uint32_t tIdx=0; tIdx<MAX_ITER; tIdx++)
        fprintf(fidProf3, "%llu\n", tData.thData.tickStoreArray[tIdx].tickSec);
    fclose(fidProf3);

    FILE *fidProf4 = fopen ("totalProfTime_n11.txt","w");
    for(uint32_t tIdx=0; tIdx<MAX_ITER; tIdx++)
        fprintf(fidProf4, "%llu\n", tData.thData.tickStoreArray[tIdx].tickNsec);
    fclose(fidProf4);


    printf("End of the main function\n"); fflush(stdout);

    /* Thread won't join since Array thread will always in busy while loop */
    pthread_join(threadArray, NULL);

    return 0;
}