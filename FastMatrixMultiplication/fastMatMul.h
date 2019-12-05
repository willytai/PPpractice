#include <pthread.h>
#define MAXTHREAD 4

// row parallelization
// returns 0 if the multiplication is successful
int fastMatMul(int Na, int Ma, int Nb, int Mb, unsigned long** A, unsigned long** B, unsigned long** C, int nthread);
