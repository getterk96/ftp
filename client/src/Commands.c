//
// Created by getterk on 17-10-29.
//

#include "Client.h"

char * judge_PASV(char * response) {
    return strstr(response, "Passive");
}

int process_PASV(char *response, struct Params *p){

    reset_file_connection(p);

    char ips[30] = {0};
    strcpy(ips, strchr(response, '(') + 1);
    *strchr(ips, ')') = '\0';
    struct IP ip = {"", 0};
    resolve_ip(ips, &ip);

    int sockfd = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        return 1;
    }

    memset(p->fileaddr, 0, sizeof(struct sockaddr_in));
    p->fileaddr->sin_family = AF_INET;
    p->fileaddr->sin_port = htons(ip.port);

    if (inet_pton(AF_INET, ip.addr, &p->fileaddr->sin_addr) <= 0) {
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)p->fileaddr, sizeof(*p->fileaddr)) < 0) {
        return 1;
    }

    *p->filefd = sockfd;

    return 0;
}

int process_PORT(struct Order *order, struct Params *p) {

    reset_file_connection(p);

    if (((*p->filelfd) = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        return 1;
    }

    struct IP ip = {"", 0};
    resolve_ip(order->parameter, &ip);

    memset(p->fileaddr, 0, sizeof(*p->fileaddr));
    p->fileaddr->sin_family = AF_INET;
    p->fileaddr->sin_port = htons(ip.port);
    p->fileaddr->sin_addr.s_addr = htonl(INADDR_ANY); //allow all ips in !!!

    if (bind(*p->filelfd, (struct sockaddr *)p->fileaddr, sizeof((*p->fileaddr))) == -1) {
        return 1;
    }

    if (listen(*p->filelfd, 1) == -1) {
        return 1;
    }

    fcntl(*p->filelfd, F_SETFL, O_NONBLOCK);

    return 0;
}

FILE * process_RETR(struct Order * order, FILE ** fp){

    if(*fp){
        fclose(*fp);
        *fp = NULL;
    }
    char filename[MAX_MESSAGE_LEN] = {0};
    strcpy(filename, order->parameter);
    *fp = fopen(filename, "wb");

    return *fp;

}

FILE * process_STOR(struct Order * order, FILE ** fp){

    if(*fp){
        fclose(*fp);
        *fp = NULL;
    }
    char filename[MAX_MESSAGE_LEN] = {0};
    strcpy(filename, order->parameter);
    *fp = fopen(filename, "rb");

    return *fp;

}

char * judge_QUIT(char * response){
    return strstr(response, "Goodbye");
}
