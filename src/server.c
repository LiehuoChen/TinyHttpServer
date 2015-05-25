//server.c
#include "server.h"

//thread func to process receive data from browser
void* getMessage(void* ptr) {
    int fd = *((int*) ptr);
    //use recv function not read function
    char buf[BUFSIZE];
    int numrecv = 0;
    if ((numrecv = recv(fd, buf, BUFSIZE, 0)) > 0) {
        buf[numrecv] = '\0';

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
        free(recbuf);
        free(path);
        free(fpath);
    } else if (numrecv == 0) {
        ;
    } else {
        close(fd);
        pthread_exit(NULL);
    }
    close(fd);
    pthread_exit(NULL);
}


void server_run(struct in_addr ip, int port, int pNum) {
    socklen_t size = 0;
    int sockfd, acceptfd;
    struct sockaddr_in seraddr, cliaddr;
    pthread_t *threads = (pthread_t *)malloc(pNum * sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "create threads error\n");
        exit(1);
    }

    //be careful with the parentheses
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(1);
    }
    //the content in sizeof should be type
    memset(&cliaddr, 0, sizeof(struct sockaddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr = ip;
    seraddr.sin_port = htons(port);
    //errno is very useful
    if (bind(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr,"bind error. errno is %d\n",errno);
        exit(1);
    }

    if (listen(sockfd, pNum) < 0) {
        fprintf(stderr,"listen error. errno is %d\n",errno);
        exit(1);
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
}
