struct message_result
{
    unsigned char version;
    unsigned char code;
    signed short flag; //Зарезервировано
    unsigned int size;
    enum : unsigned char{
        OK = 0,
        ERROR_FILENOTFOUND = 1,
        ERROR_DEPNOTFOUND = 2,
        ERROR_PKGNOTFOUND = 3,
        ERROR_PKGINCORRECT = 4,
        ERROR_CMDINCORRECT = 5
    };
};

namespace cmds
{
enum : unsigned char
{
    install = 1,
    remove = 2,
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
}
namespace flags
{
    enum : unsigned short{
        multiparams = 1 >> 0,
        old_command = 1 >> 1
    };
}
namespace cmdflags{
namespace install
{
    enum : unsigned int{
        nodep = 1 >> 0,
        fakeinstall = 1 << 1,
        full_path = 1 << 2
    };
}
}

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
extern "C"
{
extern struct ptr_and_size* source_of_package_alloc(unsigned char cmd,unsigned short flags,unsigned int cmdflags,unsigned int size);
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
}
