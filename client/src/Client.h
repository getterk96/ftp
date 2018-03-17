#ifndef CLIENT_PART_CLIENT_H
#define CLIENT_PART_CLIENT_H
//
// Created by getterk on 17-10-16.
//

#ifndef SERVER_PART_SERVER_H
#define SERVER_PART_SERVER_H

#define MAX_MESSAGE_LEN 100
#define MAX_DATA_LEN 8192

#include <sys/socket.h>
#include <netinet/in.h>
//#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <pthread.h>

struct Order {
    char header[5];
    char parameter[MAX_MESSAGE_LEN]; //the limit of the length of Order 100
};

struct IP {
    char addr[16];
    int port;
};

struct Params {
    int sockfd;
    struct sockaddr_in * fileaddr;
    int * filelfd;
    int * filefd;
    FILE ** fp;
    char * filestatus;
};

enum header { RETR, STOR, PORT, LIST };


int collect_message(char *dest, int connfd);
void * collect_index(void * args);
void * collect_file(void * args);
void * send_file(void * args);
void * send_order(void * args);

int header_judge(char *header);
int resolve_message(struct Order *order, char *source);
int send_message(char *source, int connfd);
int resolve_ip(char * ips, struct IP * ip);
int reset_file_connection(struct Params *p);

char * judge_PASV(char * response);
char * judge_QUIT(char * response);
FILE * process_RETR(struct Order * order, FILE **fp);
FILE * process_STOR(struct Order * order, FILE **fp);
int process_PORT(struct Order *order, struct Params *p);
int process_PASV(char * response, struct Params *p);


#endif //SERVER_PART_SERVER_H

#endif
