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
#define PORT 8080
#define BUFSIZE 4096
#define MAXCON 100

/* HTTP response and header for a successful request.  */

static char* ok_response =
  "HTTP/1.0 200 OK\n"
  "Content-type: text/html\n"
  "\n";

/* HTTP response, header, and body indicating that the we didn't
   understand the request.  */

static char* bad_request_response = 
  "HTTP/1.0 400 Bad Request\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Bad Request</h1>\n"
  "  <p>This server did not understand your request.</p>\n"
  " </body>\n"
  "</html>\n";

/* HTTP response, header, and body template indicating that the
   requested document was not found.  */

static char* not_found_response_template = 
  "HTTP/1.0 404 Not Found\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Not Found</h1>\n"
  "  <p>The requested URL was not found on this server.</p>\n"
  " </body>\n"
  "</html>\n";

/* HTTP response, header, and body template indicating that the
   method was not understood.  */

static char* bad_method_response_template = 
  "HTTP/1.0 501 Method Not Implemented\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Method Not Implemented</h1>\n"
  "  <p>The method %s is not implemented by this server.</p>\n"
  " </body>\n"
  "</html>\n";

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
        printf("receive: request\n");
    }
    char *recbuf = (char *)malloc(BUFSIZE * sizeof(char));
        
    if (send(fd,not_found_response_template,strlen(not_found_response_template),0) == -1) {
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
