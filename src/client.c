//client.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

//web servers use port 80
#define PORT 8080
#define BUFSIZE 4096

char *getHostname(const char* url) {
    char* path;
    char* hostname = (char *)malloc(sizeof(char)*1024);;
    int len = strlen(url);
    path = strstr(url,"/");
    int plen = strlen(path);
    int n = len - plen;
    memcpy(hostname,url,n);
    hostname[n] = '\0';

    return hostname;
}

char *geneReq(const char* url) {
    char *ret = (char *)malloc(BUFSIZE * sizeof(char));
    char* path;
    char* hostname = (char *)malloc(sizeof(char)*1024);;
    int len = strlen(url);
    path = strstr(url,"/");
    int plen = strlen(path);
    int n = len - plen;
    memcpy(hostname,url,n);
    hostname[n] = '\0';
    sprintf(ret,"GET %s HTTP/1.1\r\n",path);
    sprintf(ret + strlen(ret),"Host: %s\r\n",hostname);
    sprintf(ret + strlen(ret),"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:28.0) Gecko/20100101 Firefox/28.0\n"
                              "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
                              "Accept-Language: en-US,en;q=0.5\n"
                              "Accept-Encoding: gzip, deflate\n"
                              "Connection: keep-alive");
    
    return ret;
}

int main(int argc, char* argv[]) {
    if (argc < 1) {
        printf("You should execute like ./%s url\n",argv[0]);
    }
    int sockfd;

    struct sockaddr_in servaddr;
    struct hostent* host;
    //char recbuf[BUFSIZE];
    int len;

    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        fprintf(stderr,"sockfd created failed.\n");
        exit(-1);
    }
    memset(&servaddr,0,sizeof(struct sockaddr));
    host = gethostbyname(getHostname(argv[1]));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *) host->h_addr);
    
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(-1);
    }

    //build http request
    char* recbuf = geneReq(argv[1]);
    len = strlen(recbuf);
    //write to server
    write(sockfd,recbuf,len);
    int recnum = 0;
    //receive the response from server
    if (recnum = recv(sockfd,recbuf,BUFSIZE,0) == -1) {
        fprintf(stderr, "receive error\n");
        exit(-1);
    } else {
        printf("%s\n",recbuf);
    }
    close(sockfd);

    return 0;
}
