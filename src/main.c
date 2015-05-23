#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <netdb.h>

#include "server.h"

static const struct option long_opts [] = {
    { "address",        1, NULL, 'a'},
    { "port",           1, NULL, 'p'},
    { "thread",         1, NULL, 't'}
};

static const char* const short_opts = "a:p:t";

int main(int argc, char* argv[])
{
    int next_opt;
    struct in_addr ip;
    int port=8080;
    int pNum=1;
    ip.s_addr = INADDR_ANY;
    do {
        next_opt = getopt_long (argc, argv, short_opts, long_opts, NULL);
        switch (next_opt) {
            case 'a':
                {
                    struct hostent* ip_name;
                    ip_name = gethostbyname(optarg);
                    if (ip_name == NULL || ip_name->h_length == 0)
                        fprintf(stderr, "invalid host name.\n");
                    else
                        ip.s_addr = *((int *)(ip_name->h_addr_list[0]));
                }
                break;
             case 'p':
                port = atoi(optarg);
                break;
             case 't':
                pNum = atoi(optarg);
                break;
             case -1:
                break;
             default:
                abort();
        }
    } while (next_opt != -1);

    server_run(ip, port, pNum);
    return 0;
}
