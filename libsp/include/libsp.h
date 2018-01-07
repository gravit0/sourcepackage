struct message_head;
struct ptr_and_size;
enum cmds
{
    install = 1,
    cmds_remove = 2,
    load = 3,
    unload = 4,
    stop = 5,
    getpacks = 6,
    setconfig = 7,
    findfile = 8,
    exportfiles = 9,
    packinfo = 10,
    unloadall = 11,
    reload = 12,
    reloadall = 13,
    updateall = 14,
    config = 15,
    fixdir = 16,
    freeme = 17,
    add_listener = 18,
    remove_listener = 19,
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
extern struct ptr_and_size* source_of_package_alloc(unsigned char cmd,unsigned short flags,unsigned int cmdflags,unsigned int size);
extern struct connect_st* sp_connect_alloc(int bufsize);
extern void sp_connect_free(struct connect_st* st);
extern struct ptr_and_size source_of_package_one_command(unsigned int cmd);
extern struct ptr_and_size source_of_package_one_command_flags(unsigned int cmd,unsigned int cmdflags);
extern struct ptr_and_size source_of_package_one_command_parametr(unsigned int cmd,char* str);
extern struct ptr_and_size source_of_package_one_command_parametr_flags(unsigned int cmd,char* str,unsigned int cmdflags);
extern struct ptr_and_size source_of_package_one_command_parametr_flags_size(unsigned int cmd,char* str,unsigned int cmdflags,unsigned int lsize);
extern struct ptr_and_size sp_install(char* package);
extern struct ptr_and_size sp_remove(char* package);
extern struct ptr_and_size sp_setconfig(char* file);
extern struct ptr_and_size sp_fixdir();
extern void source_of_package_free(struct ptr_and_size data);
extern void source_of_package_freeptr(struct ptr_and_size* data);
extern int sp_connect(char* path,struct connect_st* st);
extern int sp_push_command(struct ptr_and_size* data,struct connect_st* st);
extern struct ptr_and_size sp_get_error(struct connect_st* st);
extern int sp_close(struct connect_st* st);
