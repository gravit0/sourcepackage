#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
#include <mutex>
#include <list>
#include <map>
#include "config.hpp"
#include "Logger.hpp"
#include "Sock.hpp"
#include <boost/noncopyable.hpp>
#include "package.hpp"
std::vector<std::string> split(const std::string& cmd, const char splitchar);
extern std::map<std::string, Package::ptr> packs;
extern void cmd_exec(message_head* head, std::string cmd, Client* sock);
extern void push_cmds();
extern Sock* gsock;
extern int config_parse(const std::string& filename);
int main(int argc, char** argv);
#endif
