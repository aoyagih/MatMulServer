#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

#define true 1   /* Used for fileToArray() */
#define false 0   /* Used for fileToArray() */

#define MAXSIZE 1200  /* Maximum dimention of input matrix */

int size;  /* Dimention of input matrix */
int A[MAXSIZE][MAXSIZE], B[MAXSIZE][MAXSIZE], C[MAXSIZE][MAXSIZE];   /* Matrixes which satisfy C=AB */

void DieWithError(char *errorMessage);  /* Error handling function */
void fileToArray(char fileName[], int isA);   /* Convert file to array */
void arrayToFile(char file_name[]);    /* Convert array to file */

int main(int argc, char *argv[]){
    if ((argc < 6) || (argc > 7))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port> <Matrix Size> <Input File for MatrixA> <Input File for MatrixB> [<Output File for MatrixC>]\n",
               argv[0]);
       exit(1);
    }

    int sock;                        /* Socket descriptor */
    struct sockaddr_in servAddr; /* server address */
    char *servIP;                    /* Server IP address (dotted quad) */
    unsigned short servPort;     /* server port */

    servIP = argv[1];
    servPort = atoi(argv[2]);
    size = atoi(argv[3]);

    /* Read matrix data from a file and write them to an integer array */
    fileToArray(argv[4], true);
    fileToArray(argv[5], false);


    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */
    servAddr.sin_family      = AF_INET;             /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    servAddr.sin_port        = htons(servPort); /* Server port */

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");
    
    int i, j;
    /* Send the data of array A to the server */
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data = htonl(A[i][j]);
            if (send(sock, &data, sizeof(data), 0) != sizeof(data))
                DieWithError("send() sent a different number of bytes than expected");
        }
    }
    /* Send the data of array B to the server */
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data = htonl(B[i][j]);
            if (send(sock, &data, sizeof(data), 0) != sizeof(data))
                DieWithError("send() sent a different number of bytes than expected");
        }
    }
    printf("Sent all data!\n");

    /* Recieve the data of array C from the server */
    int recvMsgSize;
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            int data;
            if ((recvMsgSize = recv(sock, &data, sizeof(data), 0)) != sizeof(data))
                DieWithError("recv() recieved a different number of bytes than expected");
            C[i][j] = ntohl(data);
        }
    }
    printf("Received all data!\n");

    /* Close the socket */
    close(sock);


    /* Write the data of array C to a file */
    if(argc == 7)
        arrayToFile(argv[6]);
    else 
        arrayToFile("matrixC.txt");


    exit(0);
    return 0;
}


/* Read matrix data from a file and write them to an integer array */
void fileToArray(char file_name[], int isA){
    int fd;  /* File descriptor */
    int buf_len = size * size * 2;
    char buf[buf_len];
    
    if((fd = open(file_name, O_RDONLY)) == -1){
        perror("open");
        exit(1);
    }

    if(read(fd, buf, buf_len) != buf_len){ 
        perror("read");
        exit(1);
    }

    char *num;
    num = strtok(buf, " \n");  /* Separate the buffer */

    int i, j;
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            if(isA){
                if (num != NULL) A[i][j] = atoi(num);
            }else{
                if (num != NULL) B[i][j] = atoi(num);
            }
            num = strtok(NULL, " \n");
        }
    }
    
    if(close(fd) == -1){
        perror("close");
        exit(1);
    }
}

/* Write the data of an integer array to a file */
void arrayToFile(char file_name[]){
    int fd;  /* File descriptor */
    int buf_len = size * size * 2;
    char buf[buf_len];
    
    if( (fd = open(file_name, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) == -1){
        perror("open");
        exit(1);
    }

    char buf_ln[1] = "\n";
    char buf_space[1] = " ";
    int i, j;
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            char buf[12];
            int digit = snprintf(buf, 12, "%d", C[i][j]);
            if(write(fd, buf, digit) != digit){ 
                perror("write");
                exit(1);
            }
            if(write(fd, buf_space, 1) != 1){ 
                perror("write");
                exit(1);
            }
        }
        if(write(fd, buf_ln, 1) != 1){ 
            perror("write");
            exit(1);
        }
    }
    
    if(close(fd) == -1){
        perror("close");
        exit(1);
    }
}