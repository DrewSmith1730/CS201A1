#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include "histogram.asmith16.h"

// global variable so all functions can access this array
int values[N];


int main(){
    // initalize the values array
    int i;
    for (i = 0; i < N; i++){
        values[i] = (int) MAXVAL * drand48();
    }

    struct timeval t1, t2, t3, t4, t5, t6;

    
    // Serial Part of main
    int serialHist[MAXVAL];
    // time stuff for serial
    gettimeofday(&t1, NULL);
    tallySerial(&serialHist);
    gettimeofday(&t2, NULL);

    printf("Time elapsed for the serial function in ms: %f \n", (t2.tv_usec - t1.tv_usec) / 1000.0);
    
    

    // Bad Multithreaded Part of main
    pthread_t badTids[NUMTHREADS];
    ThreadInfo badThreadInfo[NUMTHREADS];
    // each thread info
    int badHist[MAXVAL];

    // initalizing the badHist to have all zeros
    for(i = 0; i < MAXVAL; ++i){
        badHist[i] = 0;
    }

    // mutex set up
    pthread_mutex_t badMutex;
    pthread_mutex_init(&badMutex, NULL);

    // section length
    int len = N/NUMTHREADS;
    int sections[NUMTHREADS+1];

    // get start value of each section (+1 gives it the value N right at the end allowing for the final forloop to be much cleaner)
    for (i = 0; i < NUMTHREADS + 1; ++i){
        sections[i] = len*i;
    }
    // place each value in to the tread info it needs to be in
    for(i = 0; i < NUMTHREADS; ++i){
        badThreadInfo[i].startIndex = sections[i];
        badThreadInfo[i].endIndex = sections[i+1] - 1;
        badThreadInfo[i].histogram = &badHist;
        badThreadInfo[i].mutex = &badMutex;
    }

    // running the bad function
    gettimeofday(&t3, NULL);
    for (i = 0; i<NUMTHREADS; ++i){
        // makes all the threads used
        pthread_create(&badTids[i], NULL, tallyBad, &badThreadInfo[i]);
    }
    // waiting for all threads to end
    for (i = 0; i<NUMTHREADS; ++i){
        pthread_join(badTids[i], NULL);
    }
    gettimeofday(&t4, NULL);

    // printing output for bad function
    printf("Time in ms, for the bad formula: %f\n", (t4.tv_usec - t3.tv_usec) / 1000.0);

    // Good solution part of main
    pthread_t goodTids[NUMTHREADS];
    ThreadInfo goodThreadInfo[NUMTHREADS];

    // good hist created and set to zero in all boxes
    int goodHist[MAXVAL];
    for (i = 0; i < MAXVAL; i++){
        goodHist[i] = 0;
    }

    // mutex initialization
    pthread_mutex_t goodMutex;
    pthread_mutex_init(&goodMutex, NULL);

    // get all data for thread info
    for(i = 0; i < NUMTHREADS; ++i){
        goodThreadInfo[i].startIndex = sections[i];
        goodThreadInfo[i].endIndex = sections[i+1] - 1;
        goodThreadInfo[i].histogram = &goodHist;
        goodThreadInfo[i].mutex = &goodMutex;
    }

    // running the good thread part
    gettimeofday(&t5, NULL);
    // creating all the threads
    for(i = 0; i<NUMTHREADS; ++i){
        pthread_create(&goodTids[i], NULL, talleyGood, &goodThreadInfo[i]);
    }
    for(i = 0; i<NUMTHREADS; ++i){
        pthread_join(goodTids[i], NULL);
    }
    gettimeofday(&t6, NULL);

    // printing output for good threaded option
    printf("Good Threaded option time taken in ms: %f\n", (t6.tv_usec - t5.tv_usec) / 1000.0);

}

int tallySerial(int *histogram){
    int i, j;
    // loop the histogram to set all to zero
    for (j = 0; j < MAXVAL; ++j){
        histogram[j] = 0;
    }
    // loop the actual array to go to the spot in the histogram to increase its count
    for (i = 0; i < N; ++i){
        histogram[values[i]] += 1;
    }
    return 0;
}

void *tallyBad(void *param){
    ThreadInfo *data;
    data = (ThreadInfo *) param;
    // incrementing the histogram
    int i;
    for(i = data->startIndex; i<=data->endIndex; ++i){
        pthread_mutex_lock(data->mutex);
        data->histogram[values[i]] += 1;
        pthread_mutex_unlock(data->mutex);
    }
    
    return 0;
}

void *talleyGood(void *param){
    ThreadInfo *data;
    data = (ThreadInfo *) param;
    int localHist[MAXVAL], j;
    // init the local hist to 0
    for (j=0; j<MAXVAL; ++j){
        localHist[j] = 0;
    }
    // each value in the set of data it looks at
    for (j=data->startIndex; j<=data->endIndex; ++j){
        localHist[values[j]] += 1;
    }

    // now merge with the overall histogram
    pthread_mutex_lock(data->mutex);
    for(j = 0; j<MAXVAL; ++j){
        data->histogram[j] += localHist[j];
    }
    pthread_mutex_unlock(data->mutex);

    return 0;
}

