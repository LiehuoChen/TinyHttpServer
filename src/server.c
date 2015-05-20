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
  "HTTP/1.1 200 OK\n"
  "Content-type: text/html\n"
  "\n";

/* HTTP response, header, and body indicating that the we didn't
   understand the request.  */

static char* bad_request_response = 
  "HTTP/1.1 400 Bad Request\n"
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
  "HTTP/1.1 404 Not Found\n"
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
  "HTTP/1.1 501 Method Not Implemented\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Method Not Implemented</h1>\n"
  "  <p>The method %s is not implemented by this server.</p>\n"
  " </body>\n"
  "</html>\n";

//thread func to process receive data from browser
void* getMessage(void* ptr) {
    int fd = *((int*) ptr);
    //use recv function not read function
    char buf[BUFSIZE];
    int numrecv = 0;
    if ((numrecv = recv(fd, buf, BUFSIZE, 0)) <= 0) {
        //fprintf(stderr, "receive data error. errno is %d\n", errno);
        close(fd);
        pthread_exit(NULL);
    } else {
        buf[numrecv] = '\0';
        //printf("receive:\n%s\n",buf);
    }

    char* fpaths = strstr(buf," ");
    char* fpathe = strstr(fpaths+1," ");
    char* path = (char *)malloc(sizeof(char) * 256);
    char* fpath = (char *)malloc(sizeof(char) * 256);
    int n = strlen(fpaths) - strlen(fpathe);
    memcpy(path,fpaths+1,n-1);
    sprintf(fpath,"../html%s",path);


    char *recbuf = (char *)malloc(BUFSIZE * sizeof(char));
        //read file
    FILE* fp = fopen(fpath,"rb");
    if (fp != NULL) {
        char *filecontent = (char *)malloc(sizeof(char) * BUFSIZE);
        fseek(fp,0,SEEK_END);
        int len = ftell(fp);
        //printf("file len is %d\n",len);
        rewind(fp);
        //char *temp 
        while(fgets(filecontent+strlen(filecontent),len,fp));
        fclose(fp);
        //printf("file content-----------------------------\n%s\n",filecontent);
        sprintf(recbuf,"%s%s",ok_response,filecontent);
        //printf("recbuf is \n%s\n",recbuf);
        free(filecontent);
    } else {    
        strcpy(recbuf,not_found_response_template);
    }

    if (send(fd,recbuf,strlen(recbuf),0) == -1) {
        fprintf(stderr, "send to client error. errno is %d\n", errno);
        exit(-1);
    } else {
        printf("send success\n");
    }
    close(fd);
    free(recbuf);
    free(path);
    free(fpath);

    pthread_exit(NULL);
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
        if ((acceptfd = accept(sockfd, (struct sockaddr *)&cliaddr, &size)) == -1) {
            fprintf(stderr,"accept error. errno is %d\n", errno);
            continue;
        }
        pthread_create(&(threads[count%pNum]),NULL,getMessage,(void *)&acceptfd);
        pthread_join(threads[count%pNum],NULL);
        ++count; 
    }
    close(sockfd);

    return 0;
}
