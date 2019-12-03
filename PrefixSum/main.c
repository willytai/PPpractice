#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
 
#define MAXN 10000005
#define MAX_THREAD 2
#define DEFAULT NULL

uint32_t prefix_sum[MAXN];
uint32_t encrypted_data[MAXN];

typedef struct EncryptArg
{
    int begin, end; uint32_t key;
} EncryptArg;
static void* Encrypt(void* arg) {
    EncryptArg* args = (EncryptArg*)arg;
    int i = args->begin;
    for (; i < args->end; ++i) {
        encrypted_data[i] = encrypt(i, args->key);
    }
    pthread_exit(NULL);
}

void parallelEncrypt(pthread_t* threads, int n, uint32_t key, int blockSize) {

    // distribute tasks
    EncryptArg A[MAX_THREAD];
    for (int i = 0; i < MAX_THREAD; ++i) {
        A[i].begin = i*blockSize + 1;
        A[i].end = min(n + 1, A[i].begin + blockSize);
        A[i].key = key;
        pthread_create(&threads[i], DEFAULT, Encrypt, &A[i]);
    }

    // synchronization
    for (int i = 0; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);
}

typedef struct Range
{
    int begin, end;
} Range;
static void* PrefixSum(void* arg) {
    Range* args = (Range*)arg;
    int i = args->begin;
    prefix_sum[i] = encrypted_data[i];
    for (++i; i < args->end; ++i) {
        prefix_sum[i] = prefix_sum[i-1] + encrypted_data[i];
    }
    pthread_exit(NULL);
}
void parallelAccumulate(pthread_t* threads, int n, int blockSize) {

    // distribute tasks
    Range R[MAX_THREAD];
    for (int i = 0; i < MAX_THREAD; ++i) {
        R[i].begin = i*blockSize +1 ;
        R[i].end   = min(n + 1, R[i].begin + blockSize);
        pthread_create(&threads[i], DEFAULT, PrefixSum, &R[i]);
    }

    // synchronization
    for (int i = 0; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);
}

typedef struct OffsetArg
{
    uint32_t displacement;
    int begin, end;
} OffsetArg;
static void* Offset(void* arg) {
    const OffsetArg* args = (OffsetArg*)arg;
    int i = args->begin;
    for (; i < args->end; ++i) {
        prefix_sum[i] += args->displacement;
    }
    pthread_exit(NULL);
}
void parallelFix(pthread_t* threads, int n, int blockSize) {

    // distribute tasks
    for (int i = 1; i < MAX_THREAD; ++i) {
        const int begin = i*blockSize + 1;
        const int end   = min(n + 1, begin + blockSize);
        const int subBlockSize = ceil( (double)(end - begin) / MAX_THREAD );
        OffsetArg O[MAX_THREAD];
        for (int m = 0; m < MAX_THREAD; ++m) {
            O[m].displacement = prefix_sum[begin-1];
            O[m].begin = begin + m*subBlockSize;
            O[m].end   = min(end, O[m].begin + subBlockSize);
            pthread_create(&threads[m], DEFAULT, Offset, &O[m]);
        }

        // synchronization
        for (int i = 1; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);
    }
}

void singleThread(int n, uint32_t key) {
    uint32_t sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += encrypt(i, key);
        prefix_sum[i] = sum;
    }
}

int main() {

    int n;
    uint32_t key;

    while (scanf("%d %" PRIu32, &n, &key) == 2) {

        // create threads
        pthread_t threads[MAX_THREAD];
        int blockSize = ceil( (double)n / MAX_THREAD );

        /* parallelize encryption */
        parallelEncrypt(threads, n, key, blockSize);

        /* parallelize accumulation */
        parallelAccumulate(threads, n, blockSize);

        /* fix value by offsetting */
        parallelFix(threads, n, blockSize);

        /* print */
        output(prefix_sum, n);
    }

    // kill the main thread
    pthread_exit(NULL);

    return 0;
}
