#include "libsp.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#define SOCK_BUF 255
struct connect_st* sp_connect_alloc(int bufsize)
{
    struct connect_st* st = malloc(sizeof(struct connect_st));
    st->buf = malloc(bufsize * sizeof(char));
    return st;
}
struct ptr_and_size* sp_alloc(unsigned char cmd,unsigned short flags,unsigned int cmdflags,unsigned int size)
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
struct ptr_and_size sp_cmd(unsigned int cmd)
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
struct ptr_and_size sp_cmdf(unsigned int cmd,unsigned int cmdflags)
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
struct ptr_and_size sp_cmdp(unsigned int cmd,char* str)
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
struct ptr_and_size sp_cmdpf(unsigned int cmd,char* str,unsigned int cmdflags)
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
struct ptr_and_size sp_cmdpfs(unsigned int cmd,char* str,unsigned int cmdflags,unsigned int lsize)
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
void sp_freeptr(struct ptr_and_size* data)
{
    free(data->ptr);
    free(data);
}
void sp_free(struct ptr_and_size data)
{
    free(data.ptr);
}
int sp_connect(struct connect_st* st,char* path)
{
    int connect_res = 0;
    st->sock = 0;
    st->readed=0;
    st->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un srvr_name;
    if (st->sock < 0) {
        return st->sock;
    }
    srvr_name.sun_family = AF_UNIX;
    strcpy(srvr_name.sun_path, path);
    connect_res = connect(st->sock, (struct sockaddr*) &srvr_name, sizeof (srvr_name));
    if (connect_res < 0) {
        return connect_res - 100;
    }
    return 0;
}
int sp_close(struct connect_st* st)
{
    int val = close(st->sock);
    return val;
}
int sp_push_command(struct connect_st* st,struct ptr_and_size data)
{
    int val = 0;
    struct message_result* result;
    if(st->sock <= 0) return -2;
    send(st->sock,data.ptr,data.size,0);
    st->buf[0] = '\0';
    st->readed = recv(st->sock,st->buf,SOCK_BUF,0);
    result = (struct message_result*) st->buf;
    val=result->code;
    return val;
}
struct ptr_and_size sp_get_error(struct connect_st* st)
{
    struct ptr_and_size r;
    r.ptr = st->buf;
    r.size = st->readed;
    return r;
}
void sp_connect_free(struct connect_st* st)
{
    free(st->buf);
    free(st);
}
