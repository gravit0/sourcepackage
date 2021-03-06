struct message_head;
struct ptr_and_size;
enum cmds
{
    cmds_install = 1,
    cmds_remove = 2,
    cmds_load = 3,
    cmds_unload = 4,
    cmds_stop = 5,
    cmds_getpacks = 6,
    cmds_setconfig = 7,
    cmds_findfile = 8,
    cmds_exportfiles = 9,
    cmds_packinfo = 10,
    cmds_unloadall = 11,
    cmds_reload = 12,
    cmds_reloadall = 13,
    cmds_updateall = 14,
    cmds_config = 15,
    cmds_fixdir = 16,
    cmds_freeme = 17,
    cmds_add_listener = 18,
    cmds_remove_listener = 19,
    MAX_COMMANDS = 20
};
enum flags {
   multiparams = 1 >> 0,
   old_command = 1 >> 1,
   fullpath = 1 >> 2
};
enum flags_install{
        nodep = 1 >> 0,
        fakeinstall = 1 << 1
};
struct message_head
{
    unsigned char version;
    unsigned char cmd;
    unsigned short flag;
    unsigned int cmdflags;
    unsigned int size;
};
struct ptr_and_size
{
    void* ptr;
    unsigned int size;
};
struct ptr_and_error
{
    int sock;
    int code;
};
struct connect_st
{
    int code;
    int bufsize;
    int sock;
    int readed;
    char* buf;
};
enum RETS {
    RET_OK = 0,
    RET_ERROR_FILENOTFOUND = 1,
    RET_ERROR_DEPNOTFOUND = 2,
    RET_ERROR_PKGNOTFOUND = 3,
    RET_ERROR_PKGINCORRECT = 4,
    RET_ERROR_CMDINCORRECT = 5
};
struct message_result
{
    unsigned char version;
    unsigned char code;
    signed short flag; //Зарезервировано
    unsigned int size;
};
extern struct ptr_and_size* sp_alloc(unsigned char cmd,unsigned short flags,unsigned int cmdflags,unsigned int size);
extern struct connect_st* sp_connect_alloc(int bufsize);
extern void sp_connect_free(struct connect_st* st);
extern struct ptr_and_size sp_cmd(unsigned int cmd);
extern struct ptr_and_size sp_cmdf(unsigned int cmd,unsigned int cmdflags);
extern struct ptr_and_size sp_cmdp(unsigned int cmd,char* str);
extern struct ptr_and_size sp_cmdpf(unsigned int cmd,char* str,unsigned int cmdflags);
extern struct ptr_and_size sp_cmdpfs(unsigned int cmd,char* str,unsigned int cmdflags,unsigned int lsize);
extern void sp_free(struct ptr_and_size data);
extern void sp_freeptr(struct ptr_and_size* data);
extern int sp_connect(struct connect_st* st,char* path);
extern int sp_push_command(struct connect_st* st,struct ptr_and_size data);
extern struct ptr_and_size sp_get_error(struct connect_st* st);
extern int sp_close(struct connect_st* st);
