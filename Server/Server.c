#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define MAXSIZE 1200  /* Maximum dimention of input matrix */

void DieWithError(char *errorMessage);  /* Error handling function */
void myHandleTCPClient(int clntSocket);   /* TCP client handling function */
void *calc_product(void *arg);   /* Calculate product of two matrixes in multi-thread */


int A[MAXSIZE][MAXSIZE], B[MAXSIZE][MAXSIZE], C[MAXSIZE][MAXSIZE]; /* Matrixes which satisfy C=AB */
int size;   /* Dimention of input matrix */
int thread_count;  /* The number of thread */

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in clntAddr; /* Client address */
    unsigned short servPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */

    if (argc != 4)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port> <Matrix Size> <The Number of Thread>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]);
    size = atoi(argv[2]);
    thread_count = atoi(argv[3]);

    /* Check the number of thread */
    if(thread_count >= 10)
        DieWithError("Too many threads: The number of thread must be 0~9!");
    if(thread_count <= 0)
        DieWithError("Invalid value: The number of thread must be 0~9!");

    /* Check if array size can be divided by the number of thread */
    if(size % thread_count != 0)
        DieWithError("Array size can not be divided by the number of thread!");


    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    inet_aton("127.0.0.1", &servAddr.sin_addr);
    //servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */


    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    
    printf("Listening...\n");
    
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(clntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */

        printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

        myHandleTCPClient(clntSock);
    }
    
    /* NOT REACHED */
}


/* TCP client handling function */
void myHandleTCPClient(int clntSocket){
    int recvMsgSize;                    /* Size of received message */

    int i, j;
    /* Receive the data of array A from the client */
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data;
            if ((recvMsgSize = recv(clntSocket, &data, sizeof(data), 0)) != sizeof(data))
                DieWithError("recv() recieved a different number of bytes than expected");
            A[i][j] = ntohl(data);
        }
    }
    /* Receive the data of array B from the client */
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data;
            if ((recvMsgSize = recv(clntSocket, &data, sizeof(data), 0)) != sizeof(data))
                DieWithError("recv() recieved a different number of bytes than expected");
            B[i][j] = ntohl(data);
        }
    }

    printf("Received all data!\n");

    pthread_t th[9];   /* array of threads */
    void *rval;
    
    /* Intialize threads' name */
    char th_name[10][4];
    for(i=0; i<10; i++){
        for(j=0; j<4; j++){
            th_name[i][0] = 't';
            th_name[i][1] = 'h';
            th_name[i][2] = '0';
            th_name[i][3] = '\0';
        }
    }


    /* Get start time */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t start_sec = ts.tv_sec;
    long start_nsec = ts.tv_nsec;
    printf("start time: %10ld.%09ld\n", start_sec, start_nsec);


    /* Create multi-thread */
    for(i=0; i<thread_count; i++){
        th_name[i][2] = i + '0';
        printf("this is %s.\n", th_name[i]);
        if(pthread_create(&th[i], NULL, calc_product, (void *)&th_name[i]) != 0){
            perror("Thread creation failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    

    /* Join all threads */
    for(i=0; i<thread_count; i++){
        printf("the process joins with thread th%d\n", i);
        if(pthread_join(th[i], &rval) != 0){
            perror("Failed to join with th.\n");
        }else{
            printf("th%d (Thread No. %d) finished\n", i, *((int *)rval));
        }
    }


    /* Get end time */
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t end_sec = ts.tv_sec;
    long end_nsec = ts.tv_nsec;
    printf("end time: %10ld.%09ld\n", end_sec, end_nsec);

    /* Print exection time */
    double dif = (end_sec - start_sec) + (end_nsec - start_nsec) / 1000000000.0;
    printf("exection time:%.3fsec.\n", dif);


    /* Send the data of array C to the client */
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data = htonl(C[i][j]);
            if (send(clntSocket, &data, sizeof(data), 0) != sizeof(data))
                DieWithError("send() sent a different number of bytes than expected");
        }
    }
    printf("Sent all data!\n");

    close(clntSocket);    /* Close client socket */
}


/* Calculate product of two matrixes in multi-thread */
void *calc_product(void *arg){
    printf("in %s\n", (char *)arg);
    int i, j, k;
    int *thread_id = (int *)malloc(sizeof(int));
    *thread_id = pthread_self();

    int x = (((char *)arg)[2] - '0');
    printf("x = %d\n", x);

    int left = x*(size/thread_count);
    int right = (x+1) * (size/thread_count);
    for(i=left; i<right; i++){
        for(j=0; j<size; j++){
            int sum = 0;
            for(k=0; k<size; k++){
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    pthread_exit((void *)thread_id);
}
