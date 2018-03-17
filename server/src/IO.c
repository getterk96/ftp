//
// Created by getterk on 17-10-28.
//

#include "Server.h"

int send_message(char *source, int connfd) {

    int p = 0;
    size_t len = strlen(source);
    while (p < len) {
        ssize_t n = write(connfd, source + p, len - p);
        if (n <= 0) {
            return 1;
        } else {
            p += n;
        }
    }

    return 0;

}

int collect_message(char *dest, int connfd) {

    int p = 0;
    while (1) {
        ssize_t n = read(connfd, dest + p, MAX_MESSAGE_LEN - 1 - p);
        if (n <= 0) {
            return 1;
        } else {
            p += n;
            if (dest[p - 1] == '\n') {
                break;
            }
        }
    }

    dest[p - 1] = '\0';

    return 0;

}

void * send_file(void *args) {

    struct User * user = (struct User *)args;
    char buffer[MAX_DATA_LEN];
    size_t n = 1;
    ssize_t stat = -1;
    while (n != 0) {
        n = fread(buffer, sizeof(char), MAX_DATA_LEN, user->fp);
        stat = write(user->filefd, buffer, n);
        if(stat < 0){
            break;
        }
    }
    user->filetid = 0;
    close(user->filefd);
    user->filefd = -1;
    fclose(user->fp);
    user->fp = NULL;
    user->filecmode = -1;
    if(stat < 0){
        send_message(FILE_CONNECTION_FAILED, user->connfd);
    }
    else{
        send_message(TRANSFER_COMPLETE, user->connfd);
    };

    return NULL;

}

void * collect_file(void *args) {

    struct User *user = (struct User *) args;
    char buffer[MAX_DATA_LEN] = {0};
    ssize_t n = MAX_DATA_LEN;
    while (n == MAX_DATA_LEN) {
        n = recv(user->filefd, buffer, MAX_DATA_LEN, MSG_WAITALL);
        if(n < 0) {
            break;
        }
        fwrite(buffer, sizeof(char), n, user->fp);
    }
    user->filetid = 0;
    close(user->filefd);
    user->filefd = -1;
    fclose(user->fp);
    user->fp = NULL;
    user->filecmode = -1;
    if(n < 0){
        send_message(FILE_CONNECTION_FAILED, user->connfd);
    }
    else{
        send_message(TRANSFER_COMPLETE, user->connfd);
    };

    return NULL;

}
