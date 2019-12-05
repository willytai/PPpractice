#include "fastMatMul.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

int check(int Na, int Ma, int Nb, int Mb) {
    if (Ma == Nb) return 0;
    fprintf(stderr, "[ERROR] Unable to multiply matrix A with shape (%d, %d) and matrix B with shape (%d, %d)!\n", Na, Ma, Nb, Mb);
    fprintf(stderr, "        The second dimension of A and the first dimension of B must Match!\n");
    return -1;
}

typedef struct doRowArg
{
    int start_row; int end_row;
    int Nb; int Mb;
    unsigned long** A; unsigned long** B; unsigned long** C;
} doRowArg;
void* doRow(void* arg) {
    doRowArg* args = (doRowArg*)arg;
    for (int row = args->start_row; row < args->end_row; ++row) {
        for (int j = 0; j < args->Mb; j++) {
            unsigned long sum = 0;    // overflow, let it go.
            for (int k = 0; k < args->Nb; k++)
                sum += args->A[row][k] * args->B[k][j];
            args->C[row][j] = sum;
        }
    }
    pthread_exit(NULL);
}

int fastMatMul(int Na, int Ma, int Nb, int Mb, unsigned long** A, unsigned long** B, unsigned long** C, int nthread) {

    // check matrix
    if (check(Na, Ma, Nb, Mb) == -1) return -1;

    // check nthread
    nthread = nthread > MAXTHREAD ? MAXTHREAD : nthread;

    pthread_t threads[nthread];
    doRowArg  args[nthread];
    int blockSize = ceil( (float)Na / nthread );

    for (int tid = 0; tid < nthread; ++tid) {
        args[tid].A = A;
        args[tid].B = B;
        args[tid].C = C;
        args[tid].Nb = Nb;
        args[tid].Mb = Mb;
        args[tid].start_row = tid * blockSize;
        args[tid].end_row   = (tid + 1) * blockSize;
        args[tid].end_row   = args[tid].end_row > Na ? Na : args[tid].end_row;
        pthread_create(&threads[tid], NULL, doRow, &args[tid]);
    }

    for (int tid = 0; tid < nthread; ++tid) {
        pthread_join(threads[tid], NULL);
    }

    return 0;
}
