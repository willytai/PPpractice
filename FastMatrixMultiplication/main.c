#include <stdio.h>
#include <stdlib.h>
#include "fastMatMul.h"
 
#define DEBUG
#define UINT unsigned long 
void rand_gen(UINT c, int N, int M, UINT** A) {
    UINT x = 2, n = N*M;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            x = (x * x + c + i + j)%n;
            A[i][j] = x;
        }
    }
}
void print_matrix(int N, int M, UINT** A) {
    for (int i = 0; i < N; i++) {
        fprintf(stderr, "[");
        for (int j = 0; j < M; j++)
            fprintf(stderr, " %u", A[i][j]);
        fprintf(stderr, " ]\n");
    }
}
void allocMatrix(int N, int M, UINT*** matrix) {
    *matrix = (UINT**)malloc(N*sizeof(UINT*));
    for (int i = 0; i < N; ++i) {
        (*matrix)[i] = (UINT*)malloc(M*sizeof(UINT));
    }
}
static UINT** A;
static UINT** B;
static UINT** C;
int main() {
    int Na, Ma, Nb, Mb, S1, S2;
    while (scanf("%d %d %d %d %d %d", &Na, &Ma, &Nb, &Mb, &S1, &S2) == 6) {
        allocMatrix(Na, Ma, &A);
        allocMatrix(Nb, Mb, &B);
        allocMatrix(Na, Mb, &C);
        rand_gen(S1, Na, Ma, A);
        rand_gen(S2, Nb, Mb, B);
        if (fastMatMul(Na, Ma, Nb, Mb, A, B, C, 4) == -1) break;
#ifdef DEBUG
        print_matrix(Na, Ma, A);
        fprintf(stderr, "\n");
        print_matrix(Nb, Mb, B);
        fprintf(stderr, "\n");
        print_matrix(Na, Mb, C);
#endif
    }
    return 0;
}
