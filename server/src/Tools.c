//
// Created by getterk on 17-10-28.
//

#include "Tools.h"

int init_main_listener(struct sockaddr_in *addr, int argc, char **argv){

    prase_params(argc, argv);

    memset(users, 0, MAX_USER * sizeof(*users));
    for (int i = 0;i<10;++i){
        strcpy(users[i].cdir, ROOT);
        users[i].filefd = users[i].filelfd = users[i].connfd = users[i].filecmode = -1;
    }

    int listenfd = 0;

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(COMMAND_PORT);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ||
            bind(listenfd, (struct sockaddr *)addr, sizeof(*addr)) == -1 ||
            listen(listenfd, MAX_USER) == -1) {
        return 0;
    }

    return listenfd;
}

int check_new_connection(int listenfd){

    int connfd = accept(listenfd, NULL, NULL);

    getlocalhostip(connfd);

    int i = 0;

    for (;i<10;++i){
        if (!users[i].status) {
            users[i].connfd = connfd;
            users[i].status = 1;
            break;
        }
    }

    if (i >= 10 || pthread_create(&users[i].maintid, NULL, &main_processor, &users[i])) {
        send_message(CONNECTION_REFUSED, connfd);
        close(connfd);
        return 0;
    }

    send_message(CONNECTION_ACCEPTED, connfd);

    return connfd;

}

void * main_processor(void *args) {

    struct User * user = (struct User *)args;
    char command[MAX_MESSAGE_LEN];
    char response[MAX_MESSAGE_LEN];
    struct Order order = {"", ""};

    while(1) {

        if(collect_message(command, user->connfd)){
            strcpy(order.parameter, "QUIT");
            user_quit(user, &order);
        }

        resolve_message(command, &order);

        chdir(user->cdir);

        if(commands[header_judge(order.header)](response, &order, user)) { continue; }

        send_message(response, user->connfd);

        if(user_quit(user, &order)){
            return NULL;
        }
    }

}

int resolve_message(char *source, struct Order *order) {

    if(!strlen(source)){
        strcpy(order->header, "QUIT");
        order->parameter[0] = '\0';
        return 0;
    }

    char * part = strtok(source, " ");
    if(strchr(source, '\r')) {
        strcpy(order->header, source);
        *strchr(order->header, '\r') = '\0';
    }
    else {
        strcpy(order->header, part);
        part = strtok(NULL, " ");
        strcpy(order->parameter, part);
        *strchr(order->parameter, '\r') = '\0';
    }

    return 0;

}

int header_judge(char *header) {

    int i = 0;
    for (; i < 17; ++i) {
        if (strcmp(headers[i], header) == 0) {
            break;
        }
    }

    return i;

}

int getlocalhostip(int connfd){

    if(!strlen(LOCAL_IP)) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        getsockname(connfd, (struct sockaddr *)&addr, &len);
        sprintf(LOCAL_IP, "%s", inet_ntoa(addr.sin_addr));
        for (int i = 0;i<3;++i){
            *strchr(LOCAL_IP, '.') = ',';
        }
    }

    return 0;
}


int resolve_ip(char * ips, struct IP * ip){

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

int close_filefd(struct User *user){

    if(user->filelfd != -1) {
        close(user->filelfd);
        user->filelfd = -1;
    }
    if (user->filefd != -1) {
        close(user->filefd);
        user->filefd = -1;
    }
    if(user->filetid){
        pthread_cancel(user->filetid);
        user->filetid = 0;
    }

    if(user->fp) {
        fclose(user->fp);
        user->fp = NULL;
    }
    user->filecmode = -1;
    memset(&user->fileaddr, 0, sizeof(user->fileaddr));
    return 0;

}

int resolve_path(char *cdir, char *path){

    char buffer[MAX_MESSAGE_LEN] = {0};
    strcpy(buffer, cdir);
    if(strlen(buffer) == 0 || buffer[strlen(buffer) - 1] != '/'){
        buffer[strlen(buffer)] = '/';
    }
    if(path[0] == '/'){
        strcpy(buffer, path);
    }
    else {
        if(path[0] == '.' && path[1] == '/'){
            path += 2;
        }
        strcat(buffer, path);
        char *p = NULL;
        while((p = strstr(buffer, ".."))){
            char temp[MAX_MESSAGE_LEN] = {0};
            strcpy(temp, p + 2);
            *(p - 1) = '\0';
            char * q = strrchr(buffer, '/');
            if(!q){
                return 1;
            }
            else{
                *q = '\0';
            }
            strcat(buffer, temp);
        }
        memset(buffer + strlen(buffer), 0, MAX_MESSAGE_LEN - strlen(buffer));
        if(strlen(buffer) == 0 || buffer[strlen(buffer) - 1] != '/'){
            buffer[strlen(buffer)] = '/';
        }
    }
    if(chdir(buffer) == -1){
        chdir(cdir);
        return 1;
    }
    else{
        strcpy(cdir, buffer);
        return 0;
    }

}


int prase_params(int argc, char ** argv){

    char buffer[MAX_MESSAGE_LEN] = {0};
    if(argc == 3){
        if(!strcmp(argv[1], "-port")){
            COMMAND_PORT = atoi(argv[2]);
            strcpy(ROOT, "/tmp/");
        }
        if(!strcmp(argv[1], "-root")){
            getcwd(buffer, MAX_MESSAGE_LEN);
            resolve_path(buffer, argv[2]);
            strcpy(ROOT ,buffer);
            COMMAND_PORT = 21;
        }
    }
    else if(argc == 5) {
        if(!strcmp(argv[1], "-port")){
            COMMAND_PORT = atoi(argv[2]);
            getcwd(buffer, MAX_MESSAGE_LEN);
            resolve_path(buffer, argv[4]);
            strcpy(ROOT ,buffer);
        }
        if(!strcmp(argv[1], "-root")){
            COMMAND_PORT = atoi(argv[4]);
            getcwd(buffer, MAX_MESSAGE_LEN);
            resolve_path(buffer, argv[2]);
            strcpy(ROOT ,buffer);
        }
    }
    else {
        COMMAND_PORT = 21;
        strcpy(ROOT, "/tmp/");
    }

    return 0;

}

int user_quit(struct User *user, struct Order *o){

    if(!strcmp(o->header, "QUIT") || !strcmp(o->header, "ABOR")){
        close_filefd(user);
        close(user->connfd);
        user->connfd = -1;
        user->status = 0;
        user->maintid = 0;
        strcpy(user->cdir, ROOT);
        memset(user->ftor, 0, sizeof(user->ftor));
        return 1;
    }

    return 0;

}