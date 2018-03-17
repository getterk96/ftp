//
// Created by getterk on 17-10-29.
//

#ifndef SERVER_TOOLS_H_H
#define SERVER_TOOLS_H_H
#include "Server.h"
int (*commands[])(char *response, struct Order *o, struct User *user) = {
        process_USER,
        process_PASS,
        process_RETR,
        process_STOR,
        process_QUIT,
        process_ABOR,
        process_SYST,
        process_TYPE,
        process_PORT,
        process_PASV,
        process_MKD,
        process_CWD,
        process_LIST,
        process_RMD,
        process_RNFR,
        process_RNTO,
        process_DELE,
        process_OTHERS
};
char headers[][5] = { "USER", "PASS", "RETR", "STOR", "QUIT", "ABOR", "SYST",
                      "TYPE", "PORT", "PASV", "MKD", "CWD", "LIST", "RMD", "RNFR",
                      "RNTO", "DELE"};

#endif //SERVER_TOOLS_H_H
