#include "Client.h"

int main(int argc, char **argv) {

    int sockfd;
    int filelfd = -1;
    int filefd = -1;
    struct sockaddr_in addr, fileaddr;

    char response[MAX_MESSAGE_LEN];

    FILE * fp = NULL;
    char filestatus = -1;

    pthread_t filetid = 0;
    pthread_t commandtid = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return 2;
    }

    struct Params commandparams = {sockfd, &fileaddr, &filelfd, &filefd, &fp, &filestatus};
    while (pthread_create(&commandtid, NULL, &send_order, &commandparams)) {usleep(10);}

    while (1) {

        if (!collect_message(response, sockfd)) {
            printf("%s", response);
            if(judge_PASV(response)){
                if(process_PASV(response, &commandparams)) {
                    return 1;
                }
            }
            if(judge_QUIT(response)){
                close(sockfd);
                if(filefd != -1)
                    close(filefd);
                if(filelfd != -1)
                    close(filelfd);
                if(fp)
                    fclose(fp);
                return 0;
            }
        }

        if (filelfd != -1 && filefd == -1 && (filefd = accept(filelfd, NULL, NULL)) != -1) {}

        if (filefd != -1 && filestatus != -1 && (fp || filestatus == 2)) {
            filetid = 0;
            struct Params file = {0, NULL, &filelfd, &filefd, &fp, NULL};
            if(filestatus == 0)
                while (pthread_create(&filetid, NULL, &collect_file, &file)) {usleep(10);}
            else if (filestatus == 1)
                while (pthread_create(&filetid, NULL, &send_file, &file)) {usleep(10);}
            else
                while (pthread_create(&filetid, NULL, &collect_index, &file)) {usleep(10);}
            filestatus = -1;
        }

        usleep(100);
    }
}
