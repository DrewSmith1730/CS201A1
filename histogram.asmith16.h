#include <pthread.h>
#define MAXVAL 256
#define NUMTHREADS 4
#define N 1000000

typedef struct {
    int startIndex; // the first index in my region
    int endIndex; // the last index in my region
    int *histogram; // the global histogram
    pthread_mutex_t *mutex; // a mutex, for synchronization
} ThreadInfo;

int tallySerial(int *histogram);
void *tallyBad(void *histogram);
void *talleyGood(void *param);

