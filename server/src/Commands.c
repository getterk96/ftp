//
// Created by getterk on 17-10-28.
//

#include "Server.h"

int process_USER(char *response, struct Order *o, struct User *user){

    if(strcmp(o->parameter, "anonymous")){
        strcpy(response, WRONG_USER);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    strcmp(user->username, "anonymous");
    user->status = 2;
    strcpy(response, LOGIN_USER);

    return 0;

}

int process_PASS(char *response, struct Order *o, struct User *user){

    if(user->status != 2){
        strcpy(response, INPUT_USER_FIRST);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    /*
    if(!strchr(o->parameter, '@') ||
       !strchr(o->parameter, '.') ||
       strchr(o->parameter, '@') > strchr(o->parameter, '.')) {
        strcpy(response, WRONG_PASS);
        return 0;
    }
    */
    user->status = 3;
    strcpy(user->password, o->parameter);
    strcpy(response, LOGIN_SUCCEEDED);
    return 0;

}

int process_PORT(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    close_filefd(user);

    int sockfd;

    struct IP ip = {"", 0};
    resolve_ip(o->parameter, &ip);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        strcpy(response, FILE_CONNECTION_FAILED);
        return 0;
    }

    user->filefd = sockfd;
    memset(&user->fileaddr, 0, sizeof(struct sockaddr_in));
    user->fileaddr.sin_family = AF_INET;
    user->fileaddr.sin_port = htons(ip.port);

    if (inet_pton(AF_INET, ip.addr, &user->fileaddr.sin_addr) <= 0) {
        strcpy(response, FILE_CONNECTION_FAILED);
        return 0;
    }

    user->filecmode = 0;

    strcpy(response, "200 PORT command successful.\r\n");

    return 0;

}

int process_PASV(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    close_filefd(user);

    if ((user->filelfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        strcpy(response, FILE_CONNECTION_FAILED);
        return 0;
    }

    srand(time(NULL));
    struct IP ip = {"", rand() % 45536 + 20000};

    memset(&user->fileaddr, 0, sizeof(user->fileaddr));
    user->fileaddr.sin_family = AF_INET;
    user->fileaddr.sin_port = htons(ip.port);
    user->fileaddr.sin_addr.s_addr = htonl(INADDR_ANY); //allow all ips in !!!

    if (bind(user->filelfd, (struct sockaddr *)&user->fileaddr, sizeof(user->fileaddr)) == -1) {
        strcpy(response, FILE_CONNECTION_FAILED);
        return 0;
    }

    if (listen(user->filelfd, 1) == -1) {
        strcpy(response, FILE_CONNECTION_FAILED);
        return 0;
    }

    char addr[30] = {0};
    sprintf(addr, "%s,%d,%d", LOCAL_IP, ip.port / 256, ip.port % 256);

    fcntl(user->filelfd, F_SETFL, O_NONBLOCK);
    strcpy(response, "227 Entering Passive Mode (");
    strcat(response, addr);
    strcat(response, ").\r\n");

    user->filecmode = 1;

    return 0;

}

int process_RETR(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    if (user->filecmode == -1  ||
        user->filecmode == 0 && connect(user->filefd, (struct sockaddr *)&user->fileaddr, sizeof(user->fileaddr)) == -1||
        user->filecmode == 1 && (user->filefd = accept(user->filelfd, NULL, NULL)) == -1){
        close_filefd(user);
        strcpy(response, FILE_CONNECTION_NOT_ESTABLISHED);
        return 0;
    }

    FILE * fp = fopen(o->parameter, "rb");
    if(!fp){
        close_filefd(user);
        strcpy(response, FILE_READ_ERROR);
        return 0;
    }
    if(user->filefd) {
        strcpy(response, "150 Opening BINARY mode data connection for ");
        strcat(response, o->parameter);
        strcat(response, " (");
        struct stat filestat;
        stat(o->parameter, &filestat);
        char size[100] = {0};
        sprintf(size, "%d", (int) filestat.st_size);
        strcat(response, size);
        strcat(response, " bytes).\r\n");
        user->fp = fp;
        if (pthread_create(&users->filetid, NULL, &send_file, user)) {
            close_filefd(user);
            strcpy(response, THREAD_RUNOUT_ERROR);
        }
    }

    return 0;

}

int process_STOR(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    if (user->filecmode == -1  ||
        user->filecmode == 0 && connect(user->filefd, (struct sockaddr *)&user->fileaddr, sizeof(user->fileaddr)) == -1||
        user->filecmode == 1 && (user->filefd = accept(user->filelfd, NULL, NULL)) == -1){
        close_filefd(user);
        strcpy(response, FILE_CONNECTION_NOT_ESTABLISHED);
        return 0;
    }

    FILE * fp = fopen(o->parameter, "wb");
    if(!fp){
        close_filefd(user);
        strcpy(response, FILE_CREATE_ERROR);
        return 0;
    }
    if(user->filefd) {
        strcpy(response, "150 Opening BINARY mode data connection for ");
        strcat(response, o->parameter);
        strcat(response, ".\r\n");
        user->fp = fp;
        if (pthread_create(&users->filetid, NULL, &collect_file, user)) {
            close_filefd(user);
            strcpy(response, THREAD_RUNOUT_ERROR);
        }
    }

    return 0;

}

int process_QUIT(char *response, struct Order *o, struct User *user){

    strcpy(response, GOODBYE);

    return 0;

}

int process_ABOR(char *response, struct Order *o, struct User *user){

    strcpy(response, GOODBYE);

    return 0;

}

int process_SYST(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    strcpy(response, SYST);

    return 0;

}

int process_TYPE(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    if(!strcmp(o->parameter, "I")){
        strcpy(response, TYPE_BINARY);
    }
    else{
        strcpy(response, TYPE_OTHERS);
    }

    return 0;

}

int process_MKD(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    if(mkdir(o->parameter, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)){
        strcpy(response, DIR_OP_DENIED);
    }
    else{
        strcpy(response, DIR_CREATED);
    }

    return 0;

}

int process_CWD(char *response, struct Order *o, struct User *user) {

    if (user->status < 3) {
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    if (resolve_path(user->cdir, o->parameter)) {
        strcpy(response, DIR_OP_DENIED);
    } else {
        strcpy(response, DIR_CHANGED);
    }

    return 0;

}

int process_LIST(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }
    if (user->filecmode == -1  ||
        user->filecmode == 0 && connect(user->filefd, (struct sockaddr *)&user->fileaddr, sizeof(user->fileaddr)) == -1||
        user->filecmode == 1 && (user->filefd = accept(user->filelfd, NULL, NULL)) == -1){
        close_filefd(user);
        strcpy(response, FILE_CONNECTION_NOT_ESTABLISHED);
        return 0;
    }


    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(".")) == NULL) {
        strcpy(response, DIR_OP_DENIED);
        return 0;
    }
    send_message("150 Here comes the directory listing.\r\n", user->connfd);
    while((entry = readdir(dp)) != NULL) {
        memset(response, 0, MAX_MESSAGE_LEN);
        stat(entry->d_name,&statbuf);
        char rwx[3] = {'x','w','r'};
        if(S_ISDIR(statbuf.st_mode)) {
            if(strcmp(".",entry->d_name) == 0 ||
               strcmp("..",entry->d_name) == 0)
                continue;
            response[0] = 'd';
        }
        else
            response[0] = '-';
        unsigned short tester = 1;
        for (int i = 8;i >= 0;--i){
            if(statbuf.st_mode & ( tester << i )){
                response[9 - i] = rwx[i % 3];
            }
            else
                response[9 - i] = '-';
        }
        sprintf(response + 10, "\t%-8d", (int)statbuf.st_size);
        strcat(response, entry->d_name);
        strcat(response, "\r\n");
        send_message(response, user->filefd);
    }
    closedir(dp);
    user->filetid = 0;
    close(user->filefd);
    user->filefd = -1;
    user->filecmode = -1;
    strcpy(response, DIR_LISTEDED);

    return 0;

}

int process_RMD(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    char param[MAX_MESSAGE_LEN] = {0};
    strcat(param, "rm -r ");
    strcat(param, o->parameter);
    if(system(param) == -1){
        strcpy(response, DIR_OP_DENIED);
    }
    else{
        strcpy(response, DIR_DELETED);
    }

    return 0;

}

int process_OTHERS(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    strcpy(response, COMMAND_INVALID);
    return 0;

}

int process_RNFR(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    if(access(o->parameter, F_OK) != -1){
        strcpy(user->ftor, o->parameter);
        strcpy(response, FILE_RENAME_READY);
    }
    else{
        strcpy(response, FILE_RENAME_FAILED);
    }
    return 0;

}


int process_RNTO(char *response, struct Order *o, struct User *user){

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    if(!strlen(user->ftor)){
        strcpy(response, FILE_RENAME_FAILED);
        return 0;
    }

    char temp[MAX_MESSAGE_LEN] = {0};
    sprintf(temp, "mv %s %s", user->ftor, o->parameter);
    if(system(temp) != -1){
        strcpy(response, FILE_RENAMED);
    }
    else{
        strcpy(response, FILE_RENAME_FAILED);
    }

    memset(user->ftor, 0, sizeof(user->ftor));

    return 0;

}

int process_DELE(char *response, struct Order *o, struct User *user) {

    if(user->status < 3){
        strcpy(response, NEED_LOGIN);
        return 0;
    }
    if(user->filetid) {
        return 1;
    }

    char temp[MAX_MESSAGE_LEN] = {0};
    sprintf(temp, "rm %s", o->parameter);
    if(system(temp) != -1){
        strcpy(response, FILE_REMOVED);
    }
    else{
        strcpy(response, FILE_REMOVE_FAILED);
    }

    return 0;

}