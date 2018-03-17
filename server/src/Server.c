//
// Created by getterk on 17-10-16.
//

#include "Server.h"

int main(int argc, char **argv) {

    struct sockaddr_in addr;
    int listenfd = init_main_listener(&addr, argc, argv);
    if(!listenfd) {
        printf("Try binding port failed.\nPlease free the port and try again.");
        return 1;
    }

    while (1) { check_new_connection(listenfd); }

}
