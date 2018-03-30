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
#include "libsp.hpp"
#include "libsp_callmap.hpp"
#include <ncurses.h>
#undef OK
#define SOCK_NAME sock_path
#define BUF_SIZE 256
static const char *optString = "f:";
int main(int argc, char ** argv) {
    int opt = getopt(argc, argv, optString);
    char sock_path[BUF_SIZE];
    strcpy(sock_path,"/run/sp");
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
    buf[0] = '\0';
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
        message_head head;
        head.size = 0;
        head.cmd = 0;
        std::string inputstr(input);
        std::string arg;
        int pos = inputstr.find(' ');
        if(pos >= 0)
        {
            arg = inputstr.substr(pos + 1);
            inputstr = inputstr.substr(0,pos);
        }
        for(int i=0;i<cmds::MAX_COMMANDS;++i)
        {
            if(callmap[i] == inputstr)
            {
                head.cmd = i;
                break;
            }
        }
        void* sendbuf;
        int sendsize;
        if(arg.empty())
        {
            sendbuf = &head;
            sendsize = sizeof(head);
        }
        else{
            sendbuf = new char[sizeof(head) + arg.size()];
            head.size = arg.size();
            memcpy(sendbuf,&head,sizeof(head));
            memcpy((char*)sendbuf + sizeof(head),input + inputstr.size() + 1,arg.size());
            sendsize = sizeof(head) + arg.size();
        }
        send(sock, sendbuf, sendsize, 0);
        buf[0] = '\0';
        int res = recv(sock, buf, sizeof (buf), 0);
        if(res < 0) {
            free(input);
            break;
        }
        if(res < (signed int) sizeof(message_result))
        {
            std::cout << "Unknown result. Size: " << res << std::endl;
            free(input);
            break;
        }
        message_result* result = (message_result*)buf;
        if(result->code != message_result::OK)
        {
            if(result->code == message_result::ERROR_PKGNOTFOUND) std::cerr << "Ошибка: пакет не найден" << std::endl;
            else if(result->code == message_result::ERROR_DEPNOTFOUND) std::cerr << "Ошибка: зависимость в пакете не найдена" << std::endl;
            else if(result->code == message_result::ERROR_CMDINCORRECT) std::cerr << "Ошибка: комманда не существует или не поддерживается" << std::endl;
            else std::cerr << "Ошибка " << result->code << std::endl;
        }
        else if(head.cmd == cmds::stop)
        {
            free(input);
            break;
        }
        else if(head.cmd == cmds::getpacks && result->size > 0)
        {
            char* it = buf + sizeof(message_result);
            int len = 0;
            while(it < buf + sizeof(message_result) +  result->size - 1)
            {
                len = strlen(it+1);
                std::cout << "\x1b[33m" << std::string(it+1,len) << "\x1b[0m\t";
                if(len < 8) std::cout << "\t";
                unsigned char flag = *it;
                if(flag & 1 << 0) std::cout << "\x1b[32m"  << "[installed]" << "\x1b[0m";
                if(flag & 1 << 1) std::cout << "\x1b[31m"  << "[zombie]"  << "\x1b[0m";
                if(flag & 1 << 2) std::cout << "\x1b[36m"  << "[dep]"  << "\x1b[0m";
                std::cout << std::endl;
                it+=len+2;
            }
        }
        // Т.к. вызов readline() выделяет память, но не освобождает(а возвращает), то эту память нужно вернуть(освободить).
        free(input);
    }
    close(sock);
    return 0;
}
