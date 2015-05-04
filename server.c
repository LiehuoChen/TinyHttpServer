#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//the port must be the same in client side and server side
#define PORT 2323
#define BUFSIZE 4096
#define MAXCON 100

int main(int argc, char* argv[])
{
    int sockfd, acceptfd;
    struct sockaddr_in seraddr, cliaddr;
    char buf[BUFSIZE];
    int numrecv = 0;
    socklen_t size = 0;

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

    while (1) {
        size = sizeof(struct sockaddr_in);
        //& can be used to varaiable, not to value
        if ((acceptfd = accept(sockfd, (struct sockaddr *)&cliaddr, &size)) < 0) {
            fprintf(stderr,"accept error. errno is %d\n", errno);
            exit(-1);
        }
        //use recv not read
        if ((numrecv = recv(acceptfd, buf, BUFSIZE, 0)) < 0) {
            fprintf(stderr, "receive data error. errno is %d\n", errno);
            exit(-1);
        } else {
            buf[numrecv] = '\0';
            printf("receive: %s\n", buf);
        }
        close(acceptfd);
    }
    close(sockfd);

    return 0;
}
