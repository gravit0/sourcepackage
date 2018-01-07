#include "libsp.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
int sock;
int socksize;
#define SOCK_BUF 255
char buf[SOCK_BUF];
struct ptr_and_size* source_of_package_alloc(unsigned char cmd,unsigned short flags,unsigned int cmdflags,unsigned int size)
{
    struct message_head* head = malloc(sizeof(struct message_head));
    head->size = 0;
    head->flag = 0;
    head->cmdflags = 0;
    head->cmd = cmd;
    struct ptr_and_size* result = malloc(sizeof(struct ptr_and_size));
    result->ptr = head;
    result->size = sizeof(struct message_head);
    return result;
}
struct ptr_and_size source_of_package_one_command(unsigned int cmd)
{
    struct message_head* head = malloc(sizeof(struct message_head));
    head->size = 0;
    head->flag = 0;
    head->cmdflags = 0;
    head->cmd = cmd;
    struct ptr_and_size result;
    result.ptr = head;
    result.size = sizeof(struct message_head);
    return result;
}
struct ptr_and_size source_of_package_one_command_flags(unsigned int cmd,unsigned int cmdflags)
{
    struct message_head* head = malloc(sizeof(struct message_head));
    head->size = 0;
    head->flag = 0;
    head->cmdflags = cmdflags;
    head->cmd = cmd;
    struct ptr_and_size result;
    result.ptr = head;
    result.size = sizeof(struct message_head);
    return result;
}
struct ptr_and_size source_of_package_one_command_parametr(unsigned int cmd,char* str)
{
    int lsize;
    struct message_head* head;
    lsize = strlen(str);
    head = malloc(sizeof(struct message_head) + lsize);
    head->size = lsize;
    head->flag = 0;
    head->cmdflags = 0;
    head->cmd = cmd;
    memcpy((void*)head + sizeof(struct message_head),str,lsize);
    struct ptr_and_size result;
    result.ptr = head;
    result.size = sizeof(struct message_head) + lsize;
    return result;
}
struct ptr_and_size source_of_package_one_command_parametr_flags(unsigned int cmd,char* str,unsigned int cmdflags)
{
    int lsize;
    struct message_head* head;
    lsize = strlen(str);
    head = malloc(sizeof(struct message_head) + lsize);
    head->size = lsize;
    head->flag = 0;
    head->cmdflags = cmdflags;
    head->cmd = cmd;
    memcpy((void*)head + sizeof(struct message_head),str,lsize);
    struct ptr_and_size result;
    result.ptr = head;
    result.size = sizeof(struct message_head) + lsize;
    return result;
}
struct ptr_and_size source_of_package_one_command_parametr_flags_size(unsigned int cmd,char* str,unsigned int cmdflags,unsigned int lsize)
{
    struct message_head* head;
    head = malloc(sizeof(struct message_head) + lsize);
    head->size = lsize;
    head->flag = 0;
    head->cmdflags = cmdflags;
    head->cmd = cmd;
    memcpy((void*)head + sizeof(struct message_head),str,lsize);
    struct ptr_and_size result;
    result.ptr = head;
    result.size = sizeof(struct message_head) + lsize;
    return result;
}
struct ptr_and_size sp_install(char* package)
{
    return source_of_package_one_command_parametr(install,package);
}
struct ptr_and_size sp_remove(char* package)
{
    return source_of_package_one_command_parametr(cmds_remove,package);
}
struct ptr_and_size sp_setconfig(char* file)
{
    return source_of_package_one_command_parametr(setconfig,file);
}
struct ptr_and_size sp_fixdir()
{
    return source_of_package_one_command(fixdir);
}
void source_of_package_freeptr(struct ptr_and_size* data) 
{
    free(data->ptr);
    free(data);
}
void source_of_package_free(struct ptr_and_size data) 
{
    free(data.ptr);
}
int sp_connect(char* path)
{
    int connect_res = 0;
    sock = 0;
    socksize=0;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un srvr_name;
    if (sock < 0) {
        return sock;
    }
    srvr_name.sun_family = AF_UNIX;
    strcpy(srvr_name.sun_path, path);
    connect_res = connect(sock, (struct sockaddr*) &srvr_name, sizeof (srvr_name));
    if (connect_res < 0) {
        return connect_res - 100;
    }
    return 0;
}
int sp_close()
{
    int val = close(sock);
    sock = -1;
    return val;
}
int sp_push_command(struct ptr_and_size* data)
{
    int val = -3;
    if(sock <= 0) return -2;
    send(sock,data->ptr,data->size,0);
    buf[0] = '\0';
    socksize = recv(sock,buf,SOCK_BUF,0);
    if(buf[0] == '0') val=0;
    else val=-1;
    source_of_package_free(*data);
    return val;
}
struct ptr_and_size sp_get_error()
{
    struct ptr_and_size r;
    r.ptr = buf;
    r.size = socksize;
    return r;
}
