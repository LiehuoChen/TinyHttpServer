//server.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//the port must be the same in client side and server side
#define PORT 2323
#define BUFSIZE 4096
#define MAXCON 100


void* getMessage(void* ptr) {
    int fd = *((int*) ptr);
    //use recv not read
    char buf[BUFSIZE];
    int numrecv = 0;
    if ((numrecv = recv(fd, buf, BUFSIZE, 0)) < 0) {
        fprintf(stderr, "receive data error. errno is %d\n", errno);
        exit(-1);
    } else {
        buf[numrecv] = '\0';
        printf("receive: %s\n", buf);
    }
    char *recbuf = (char *)malloc(BUFSIZE * sizeof(char));
    sprintf(recbuf,"%s has received.\n",buf);
    if (send(fd,recbuf,strlen(recbuf),0) == -1) {
        fprintf(stderr, "send to client error. errno is %d\n", errno);
        exit(-1);
    } else {
        printf("send success\n");
    }
    free(recbuf);
}

int main(int argc, char* argv[])
{
    if (argc < 1) {
        printf("You should execute like ./%s 5, 5 is how many threads can create\n", argv[0]);
        exit(-1);
    }
    int sockfd, acceptfd;
    struct sockaddr_in seraddr, cliaddr;
    //char buf[BUFSIZE];
    //int numrecv = 0;
    socklen_t size = 0;
    int pNum = atoi(argv[1]);
    pthread_t *threads = (pthread_t *)malloc(pNum * sizeof(pthread_t));

    //be careful with the parentheses
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(-1);
    }
    //the content in sizeof should be type
    memset(&cliaddr, 0, sizeof(struct sockaddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(PORT);
    //errno is very useful
    if (bind(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr,"bind error. errno is %d\n",errno);
        exit(-1);
    }

    if (listen(sockfd, MAXCON) < 0) {
        fprintf(stderr,"listen error. errno is %d\n",errno);
        exit(-1);
    }

    int count = 0;
    while (1) {
        size = sizeof(struct sockaddr_in);
        //& can be used to varaiable, not to value
        if ((acceptfd = accept(sockfd, (struct sockaddr *)&cliaddr, &size)) < 0) {
            fprintf(stderr,"accept error. errno is %d\n", errno);
            exit(-1);
        }
       
        pthread_create(&(threads[count%pNum]),NULL,getMessage,(void *)&acceptfd);
        pthread_join(threads[count%pNum],NULL);
        ++count; 
        close(acceptfd);
    }
    close(sockfd);

    return 0;
}
