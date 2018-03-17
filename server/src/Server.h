//
// Created by getterk on 17-10-16.
//

#ifndef SERVER_PART_SERVER_H
#define SERVER_PART_SERVER_H

#define MAX_USER 10
#define MAX_MESSAGE_LEN 1024
#define MAX_NAME_PASSWORD_LEN 20
#define MAX_DATA_LEN 8192


#define CONNECTION_REFUSED "421 Connection refused.\r\n"
#define CONNECTION_ACCEPTED "220 Anonymous FTP server ready.\r\n"
#define COMMAND_INVALID "550 Invalid command.\r\n"
#define WRONG_USER "530 Only anonymous users are permitted.\r\n"
//#define WRONG_PASS "530 Login incorrect, invalid email format.\r\n"
#define NEED_LOGIN "530 Please login with USER and PASS.\r\n"
#define LOGIN_USER "331 Guest login ok, send your complete e-mail address as password.\r\n"
#define INPUT_USER_FIRST "503 Login with USER first.\r\n"
#define LOGIN_SUCCEEDED "230 Guest login successful.\r\n"

#define TRANSFER_COMPLETE "226 Transfer complete.\r\n"
#define FILE_CONNECTION_FAILED "421 Connection failed.\r\n"
#define FILE_CONNECTION_NOT_ESTABLISHED "425 Connection not established.\r\n"
#define FILE_READ_ERROR "551 File reading failed.\r\n"
#define FILE_CREATE_ERROR "551 File creating failed.\r\n"
#define THREAD_RUNOUT_ERROR "551 Thread creating failed.\r\n"

#define GOODBYE "221 Goodbye.\r\n"
#define TYPE_BINARY "200 Type set to I.\r\n"
#define TYPE_OTHERS "502 Not supported Type.\r\n"
#define SYST "215 UNIX Type: L8\r\n"

#define DIR_OP_DENIED "550 Permission denied.\r\n"
#define DIR_CREATED "250 Directory successfully created.\r\n"
#define DIR_CHANGED "250 Directory successfully changed.\r\n"
#define DIR_DELETED "250 Directory successfully deleted.\r\n"
#define DIR_LISTEDED "226 Directory send OK.\r\n"

#define FILE_RENAME_READY "350 Ready for RNTO.\r\n"
#define FILE_RENAMED "250 Rename successful.\r\n"
#define FILE_RENAME_FAILED "550 RNFR command failed.\r\n"

#define FILE_REMOVED "250 Delete operation successful.\r\n"
#define FILE_REMOVE_FAILED "550 Delete operation failed.\r\n"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>

struct Order {
    char header[5];
    char parameter[MAX_MESSAGE_LEN]; //the limit of the length of Order 100
};

struct IP {
    char addr[16];
    int port;
};

struct User {

    char cdir[MAX_MESSAGE_LEN];
    char username[MAX_NAME_PASSWORD_LEN];
    char password[MAX_NAME_PASSWORD_LEN];
    char ftor[MAX_MESSAGE_LEN];
    int  connfd;
    int  filefd;
    int  filelfd;
    char filecmode;
    pthread_t maintid;
    pthread_t filetid;
    FILE * fp;
    char status;
    struct sockaddr_in fileaddr;

} users[10]; // status : authenticated : 3, logged in : 2, connected : 1, logged out : 0,

int COMMAND_PORT;
char LOCAL_IP[16];
char ROOT[100];

int process_USER(char *response, struct Order *o, struct User *user);
int process_PASS(char *response, struct Order *o, struct User *user);
int process_RETR(char *response, struct Order *o, struct User *user);
int process_STOR(char *response, struct Order *o, struct User *user);
int process_QUIT(char *response, struct Order *o, struct User *user);
int process_ABOR(char *response, struct Order *o, struct User *user);
int process_SYST(char *response, struct Order *o, struct User *user);
int process_TYPE(char *response, struct Order *o, struct User *user);
int process_PORT(char *response, struct Order *o, struct User *user);
int process_PASV(char *response, struct Order *o, struct User *user);
int process_MKD(char *response, struct Order *o, struct User *user);
int process_CWD(char *response, struct Order *o, struct User *user);
int process_LIST(char *response, struct Order *o, struct User *user);
int process_RMD(char *response, struct Order *o, struct User *user);
int process_RNFR(char *response, struct Order *o, struct User *user);
int process_RNTO(char *response, struct Order *o, struct User *user);
int process_DELE(char *response, struct Order *o, struct User *user);
int process_OTHERS(char *response, struct Order *o, struct User *user);

int check_new_connection(int listenfd);
int close_filefd(struct User *user);
int init_main_listener(struct sockaddr_in * addr, int argc, char **argv);
int collect_message(char * dest, int connfd);
int resolve_message(char * source, struct Order * order);
int header_judge(char * header);
int send_message(char * source, int connfd);

void * main_processor(void * args);
void * collect_file(void * args);
void * send_file(void * args);

int getlocalhostip(int connfd);
int resolve_ip(char * ips, struct IP * ip);
int resolve_path(char *cdir, char *path);
int prase_params(int argc, char ** argv);
int user_quit(struct User *user, struct Order *o);

#endif //SERVER_PART_SERVER_H

