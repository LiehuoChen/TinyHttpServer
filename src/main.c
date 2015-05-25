#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <netdb.h>

#include "server.h"

static const struct option long_opts [] = {
    { "address",        1, NULL, 'a'},
    { "port",           1, NULL, 'p'},
    { "maxconn",         1, NULL, 'm'}
};

static const char* const short_opts = "a:p:t";

int main(int argc, char* argv[])
{
    if (argc < 7) {
        printf("You should execute like:\n %s --address xxx --port xxxx --maxconn x\n",argv[0]);
    }
    int next_opt;
    struct in_addr ip;
    int port=8080;
    int maxconn=1;
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
             case 'm':
                maxconn = atoi(optarg);
                break;
             case -1:
                break;
             default:
                abort();
        }
    } while (next_opt != -1);
    printf("port is %d, maxconn is %d\n",port,maxconn);
    server_run(ip, port, maxconn);
    return 0;
}
