//
// Created by getterk on 17-10-29.
//

#include "Client.h"

char header_names[4][5] = {"RETR", "STOR", "PORT", "LIST"};

void * send_order(void * args){

    struct Params * p = (struct Params *)args;
    char command[MAX_MESSAGE_LEN];
    struct Order order = {"", ""};
    while(1) {

        fgets(command, MAX_MESSAGE_LEN, stdin);
        if (resolve_message(&order, command)) {
            continue;
        }

        switch (header_judge(order.header)) {
            case RETR:
                if (!process_RETR(&order, p->fp)) {
                    continue;
                }
                *p->filestatus = 0;
                break;
            case STOR:
                if (!process_STOR(&order, p->fp)) {
                    continue;
                }
                *p->filestatus = 1;
                break;
            case LIST:
                if(*p->fp) {
                    fclose(*p->fp);
                    *p->fp = NULL;
                }
                *p->filestatus = 2;
                break;
            case PORT:
                if (process_PORT(&order, p))
                    continue;
                break;
            default:
                break;
        }

        command[strlen(command) + 1] = '\0';
        command[strlen(command) - 1] = '\r';
        command[strlen(command)] = '\n';
        send_message(command, p->sockfd);

        if(!strcmp(order.header, "QUIT"))
            return NULL;
    }
}

int header_judge(char *header) {
    for (int i = 0; i < 13; ++i) {
        if (strcmp(header_names[i], header) == 0) {
            return i;
        }
    }
}

int resolve_message(struct Order *order, char *source) {

    char temp[MAX_MESSAGE_LEN] = {0};
    strcpy(temp, source);
    char * part = strtok(temp, " ");
    if(strchr(temp, '\n')) {
        strcpy(order->header, temp);
        *strchr(order->header, '\n') = '\0';
    }
    else {
        strcpy(order->header, part);
        part = strtok(NULL, " ");
        strcpy(order->parameter, part);
        *strchr(order->parameter, '\n') = '\0';
    }

    return 0;
}

int resolve_ip(char * ips, struct IP * ip){ //unjudged the legalness of ip

    char temp[30] = {0};
    strcpy(temp, ips);
    char * part = strtok(temp, ",");
    int i = 0;
    for(;i < 4; ++i)
    {
        strcat(ip->addr, part);
        strcat(ip->addr, ".");
        part = strtok(NULL, ",");
    }
    ip->addr[strlen(ip->addr) - 1] = '\0';
    if(!part)
        ip->port = 0;
    else{
        ip->port = atoi(part) * 256;
        ip->port += atoi(strtok(NULL, ","));
    }

    return 0;

}

int reset_file_connection(struct Params *p){
    if(*p->filelfd != -1){
        close(*p->filelfd);
        *p->filelfd = -1;
    }
    if(*p->filefd != -1){
        close(*p->filefd);
        *p->filefd = -1;
    }
    if(*p->fp){
        fclose(*p->fp);
        *p->fp = NULL;
    }
    if(*p->filestatus >= 0){
        *p->filestatus = -1;
    }

}