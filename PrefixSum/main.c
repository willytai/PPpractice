#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
 
#define MAXN 10000005
#define MAX_THREAD 4
#define DEFAULT NULL

uint32_t prefix_sum[MAXN];
uint32_t check[MAXN];
uint32_t encrypted_data[MAXN];

typedef struct EncryptArg
{
    int begin, end; uint32_t key;
} EncryptArg;
static void* Encrypt(void* arg) {
    EncryptArg* args = (EncryptArg*)arg;
    printf("encrypt range: %d ~ %d\n", args->begin, args->end);
    int i = args->begin;
    for (; i < args->end; ++i) {
        encrypted_data[i] = encrypt(i, args->key);
    }
}

typedef struct Range
{
    int begin, end;
} Range;
static void* PrefixSum(void* arg) {
    Range* args = (Range*)arg;
    printf("presum range: %d ~ %d\n", args->begin, args->end);
    int i = args->begin;
    prefix_sum[i] = encrypted_data[i+1]; i++;
    for (; i < args->end; ++i) {
        prefix_sum[i] += prefix_sum[i-1] + encrypted_data[i+1];
    }
}

typedef struct OffsetArg
{
    uint32_t displacement;
    int begin, end;
} OffsetArg;
static void* Offset(void* arg) {
    OffsetArg* args = (OffsetArg*)arg;
    printf("offset range: %d ~ %d\n", args->begin, args->end);
    int i = args->begin;
    for (; i < args->end; ++i) {
        prefix_sum[i] += args->displacement;
    }
}

int main() {

    int n;
    uint32_t key;

    while (scanf("%d %" PRIu32, &n, &key) == 2) {

        // create threads
        pthread_t threads[MAX_THREAD];

        /* parallelize encryption */

        // distribute tasks
        int blockSize = ceil( (float)n / MAX_THREAD );
        EncryptArg A[MAX_THREAD];
        for (int i = 0; i < MAX_THREAD; ++i) {
            A[i].begin = i*blockSize;
            A[i].end = min(n, A[i].begin + blockSize);
            A[i].key = key;
            pthread_create(&threads[i], DEFAULT, Encrypt, &A[i]);
            printf("thread %d created with %d tasks\n", i, A[i].end - A[i].begin);
        }

        // synchronization
        for (int i = 0; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);

        /* parallelize accumulation */

        // distribute tasks
        Range R[MAX_THREAD];
        for (int i = 0; i < MAX_THREAD; ++i) {
            R[i].begin = i*blockSize;
            R[i].end   = min(n, R[i].begin + blockSize);
            pthread_create(&threads[i], DEFAULT, PrefixSum, &R[i]);
        }

        // synchronization
        for (int i = 0; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);


        /* fix value by offsetting */

        // distribute tasks
        for (int i = 1; i < MAX_THREAD; ++i) {
            const int begin = i*blockSize;
            const int end   = min(n, begin + blockSize);
            const int subBlockSize = ceil( (float)(end - begin) / MAX_THREAD );
            OffsetArg O[MAX_THREAD];
            for (int m = 0; m < MAX_THREAD; ++m) {
                O[m].displacement = prefix_sum[i*blockSize-1];
                O[m].begin = begin + m*subBlockSize;
                O[m].end   = min(end, O[m].begin + subBlockSize);
                pthread_create(&threads[m], DEFAULT, Offset, &O[m]);
            }

            // synchronization
            for (int i = 1; i < MAX_THREAD; ++i) pthread_join(threads[i], DEFAULT);

            printf("\n");
        }

        uint32_t sum = 0;
        for (int i = 1; i <= n; i++) {
            sum += encrypt(i, key);
            check[i] = sum;
        }

        for (int i = 1; i <= n; i++) {
            printf("%d ", check[i]);
        } printf("\n");
        for (int i = 1; i <= n; i++) {
            printf("%d ", prefix_sum[i]);
        } printf("\n");

        output(prefix_sum, n);
    }
    return 0;
}