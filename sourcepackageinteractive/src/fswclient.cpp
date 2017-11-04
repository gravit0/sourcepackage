/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fswclient.cpp
 * Author: gravit
 *
 * Created on 23 октября 2017 г., 14:29
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include "basefunctions.h"
#include <sys/un.h>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>
#define SOCK_NAME sock_path
#define BUF_SIZE 256

std::vector<std::string> split(const std::string cmd, const char splitchar) {
    int opos = 0;
    std::vector<std::string> list;
    while (true) {
        bool tr = false;
        int pos = findNoSlash(cmd, splitchar, opos, &tr);
        if (pos <= 0) {
            std::string value = cmd.substr(opos, pos - cmd.size());
            if (tr) SlashReplace(&value, 0);
            list.push_back(value);
            break;
        }
        std::string value = cmd.substr(opos, pos - opos);
        list.push_back(value);
        opos = pos + 1;
    }
    return list;
}
static const char *optString = "f:";

struct Args {
    bool flagInstall = false;
    bool flagRemove = false;
    bool flagLoad = false;
    bool flagGetpack = false;
    bool flagStop = false;
    bool flagSetConfig = false;
    std::string pkgname;
    bool flagU = false;
};

int main(int argc, char ** argv) {
    int opt = getopt(argc, argv, optString);
    char sock_path[BUF_SIZE];
    strcpy(sock_path,"/run/sp");
    Args args;
    int init_path = 0;
    char buf[BUF_SIZE];
    buf[0] = 0;
    while (opt != -1) {
        switch (opt) {
            case 'f':
            {
                strcpy(sock_path, optarg);
                init_path = 1;
                break;

            }
            default:
                /* сюда на самом деле попасть невозможно. */
                break;
        }
        opt = getopt(argc, argv, optString);
    }
    int sock;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un srvr_name;
    if (sock < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    srvr_name.sun_family = AF_UNIX;
    if (init_path) strcpy(srvr_name.sun_path, SOCK_NAME);
    else strcpy(srvr_name.sun_path, "/run/sp");
    if (connect(sock, (sockaddr*) &srvr_name, sizeof (srvr_name)) < 0) {
        perror("connect failed");
        exit(2);
    }
    char* input, shell_prompt[100];
    send(sock, "freeme", 6, 0);
    buf[0] = '\0';
    recv(sock, buf, sizeof (buf), 0);
    for(;;)
    {
        // getting the current user 'n path
        snprintf(shell_prompt, sizeof(shell_prompt), "%s@%s $ ", SOCK_NAME , getenv("USER"));
        // inputing...
        input = readline(shell_prompt);
        // eof
        if (input==NULL)
            break;
        if(!strcmp(input,"exit")) break;
	    // path autocompletion when tabulation hit
        rl_bind_key('\t', rl_complete);
        // adding the previous input into history
        add_history(input);
        send(sock, input, strlen(input), 0);
        buf[0] = '\0';
        recv(sock, buf, sizeof (buf), 0);
        if (buf[0] != '\0') {
            std::string cmd(buf);
            std::vector<std::string> args = split(cmd, ' ');
            if (args[0] != "0") std::cout << cmd << std::endl;
        }
        /* do stuff */

        // Т.к. вызов readline() выделяет память, но не освобождает(а возвращает), то эту память нужно вернуть(освободить).
        free(input);
    }
    close(sock);
    return 0;
}
