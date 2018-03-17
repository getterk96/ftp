//
// Created by getterk on 17-10-29.
//

#include "Client.h"

int collect_message(char *dest, int connfd) {
    int p = 0;
    while (1) {
        int n = read(connfd, dest + p, MAX_MESSAGE_LEN - 1 - p);
        if (n < 0) {
            //printf("Error read(): %s(%d)\n", strerror(errno), errno);
            return 1;
        } else if (n == 0) {
            break;
        } else {
            p += n;
            if (dest[p - 1] == '\n') {
                break;
            }
        }
    }
    dest[p - 2] = '\n';
    dest[p - 1] = '\0';
    return 0;
}

int send_message(char *source, int connfd) {
    int p = 0;
    int len = strlen(source);
    while (p < len) {
        int n = write(connfd, source + p, len - p);
        if (n < 0) {
            //printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return 1;
        } else {
            p += n;
        }
    }
}

void * collect_file(void * args) {

    struct Params *file = (struct Params *) args;
    char buffer[MAX_DATA_LEN] = {0};
    ssize_t n = MAX_DATA_LEN;
    while (n == MAX_DATA_LEN) {
        n = recv(*file->filefd, buffer, MAX_DATA_LEN, MSG_WAITALL);
        if(n < 0) {
            break;
        }
        fwrite(buffer, sizeof(char), n, *(file->fp));
    }
    if (*file->filelfd != -1)
        close(*file->filelfd);
    close(*file->filefd);
    *file->filelfd = *file->filefd = -1;
    fclose(*file->fp);
    *file->fp = NULL;

}

void * send_file(void * args) {

    struct Params * file = (struct Params *)args;
    char buffer[MAX_DATA_LEN];
    size_t n = 1;
    while (n != 0) {
        n = fread(buffer, sizeof(char), MAX_DATA_LEN, *file->fp);
        ssize_t stat = write(*file->filefd, buffer, n);
        if(stat < 0){
            break;
        }
    }
    if(*file->filelfd != -1)
        close(*file->filelfd);
    close(*file->filefd);
    *file->filefd = *file->filelfd = -1;
    fclose(*file->fp);
    *file->fp = NULL;

}

void * collect_index(void * args) {
    struct Params * file = (struct Params *)args;
    char buffer[MAX_DATA_LEN] = {0};
    while (1) {
        int n = (int)read(*(file->filefd), buffer, MAX_DATA_LEN - 1);
        if (n <= 0) {
            if (*file->filelfd != -1)
                close(*file->filelfd);
            close(*file->filefd);
            *file->filelfd = *file->filefd = -1;
            return NULL;
        } else {
            buffer[n - 2] = '\n';
            buffer[n - 1] = '\0';
            printf("%s", buffer);
        }
    }
}
