#include "server.h"

int make_socket_non_block(int sfd) {
    int flag, rval;
    flag = fcntl(sfd, F_GETFL, 0);
    if (flag == -1) {
        perror("fcntl");
        return -1;
    }
    flag |= O_NONBLOCK;
    rval = fcntl(sfd, F_SETFL, flag);
    if (rval == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

void process_request(int fd) {
    //use recv function not read function
    char buf[BUFSIZE];
    int numrecv = 0;
    do {
        numrecv = recv(fd, buf, BUFSIZE, 0);
        if(numrecv > 0) {
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
                rewind(fp);
                while(fgets(filecontent+strlen(filecontent),len,fp));
                fclose(fp);
                sprintf(recbuf,"%s%s",ok_response,filecontent);
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
        }
    } while(errno != EAGAIN);
}

void server_run(struct in_addr ip, int port, int maxconn) {
    int sfd,efd;
    int rval;   //use to check return value
    struct epoll_event event;
    struct epoll_event* events;
    struct sockaddr_in server_addr;

    if ((sfd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket error\n");
        abort();
    }
    memset(&server_addr, 0, sizeof (struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = ip;
    server_addr.sin_port = htons(port);

    if ((rval = bind(sfd, &server_addr, sizeof(server_addr))) < 0) {
        perror("bind error\n");
        abort();
    }
    if ((rval = make_socket_non_block(sfd)) == -1) {
        perror("make socket non block error\n");
        abort();
    }

    if (listen(sfd, maxconn) < 0) {
        perror("listen error\n");
        abort();
    }
    if ((efd = epoll_create1(0)) < 0) {
        perror("epoll create error\n");
        abort();
    }
    //set the socketfd to event fd to monitor
    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    if((rval = epoll_ctl(efd,EPOLL_CTL_ADD, sfd, &event)) < 0) {
        perror("epoll_ctl error 1\n");
        abort();
    }
    events = calloc(maxconn,sizeof(event));
    //calloc vs malloc
    //calloc will init the memory all to zero
    //malloc won't, so malloc is faster than calloc

    while (1) {
        int n,i;
        n = epoll_wait(efd, events, maxconn, -1);
        for (i = 0; i < n; ++i) {
            if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            } else if (sfd == events[i].data.fd) {
                /* we have a notification on the listening socket, which means
                one or more incoming connections*/
                while (1) {
                    int acceptfd;
                    char hbuf[80], sbuf[80];
                    struct sockaddr client_addr;
                    socklen_t size = sizeof(struct sockaddr);
                    if ((acceptfd = accept(sfd, &client_addr, &size)) == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                            //we have processed all connections
                            break;
                        else {
                            perror("accept error\n");
                            break;
                        }
                    }
                    rval = getnameinfo (&client_addr, size,
                            hbuf, sizeof(hbuf),
                            sbuf, sizeof(sbuf),
                            NI_NUMERICHOST | NI_NUMERICSERV);
                    if (rval == 0)
                    {
                        printf("Accepted connection on descriptor %d "
                                "(host=%s, port=%s)\n", acceptfd, hbuf, sbuf);
                    }

                    if ((rval = make_socket_non_block(acceptfd)) < 0)
                        abort();

                    event.data.fd = acceptfd;
                    event.events = EPOLLIN | EPOLLET;

                    if((rval = epoll_ctl(efd,EPOLL_CTL_ADD, acceptfd, &event)) < 0) {
                        perror("epoll_ctl error 2\n");
                        abort();
                    }
                }
                continue;
            } else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                process_request(events[i].data.fd);
                close(events[i].data.fd);
            }
        }
    }

    free(events);
    close(sfd);
}
