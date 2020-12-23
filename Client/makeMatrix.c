/* Useful program for making matrix file */

/* How to execute */
/* ./makeMatrix 100 > output.txt */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1200

int main(int argc, char *argv[]){
    int n = atoi(argv[1]);
    int A[N][N];
    int i, j;

    srand((unsigned int)time(NULL));

    for(i=0; i<n; i++){
        for(j=0; j<n-1; j++){
            A[i][j] = rand()%10;
            printf("%d ", A[i][j]);
        }
        A[i][n-1] = rand()%10;
        printf("%d\n", A[i][n-1]);
    }

    return 0;
}