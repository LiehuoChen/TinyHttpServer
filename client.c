//client.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 2323
#define BUFSIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 1) {
        printf("You should execute like ./%s ip\n",argv[0]);
    }
    int sockfd;

    struct sockaddr_in servaddr;
    char recbuf[BUFSIZE];
    int len;

    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        fprintf(stderr,"sockfd created failed.\n");
        exit(-1);
    }
    memset(&servaddr,0,sizeof(struct sockaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    //servaddr.sin_addr.s_addr = inet_addr("192.168.1.106");
    //use pton, the return value will give you more information
    if (inet_pton(AF_INET,argv[1],&(servaddr.sin_addr.s_addr)) <= 0) {
        fprintf(stderr,"inet_pton error\n");
        exit(-1);
    }
    
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(-1);
    }

    printf("Please input what you want to send to server:\n");
    scanf("%s",recbuf);
    len = strlen(recbuf);
    write(sockfd,recbuf,len);
    int recnum = 0;
    if (recnum = recv(sockfd,recbuf,BUFSIZE,0) == -1) {
        fprintf(stderr, "receive error\n");
        exit(-1);
    } else {
        printf("%s\n",recbuf);
    }
    close(sockfd);

    return 0;
}
