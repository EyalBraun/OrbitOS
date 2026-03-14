#ifndef COMMANDS_H
#define COMMANDS_H

#include "FileSystem.h"
#include <string>
#include <vector>

#define COMMAND_LIST \
    X(help)   X(ls)    X(mkdir) \
    X(cd)     X(save)  X(load)  \
    X(tree)   X(wtf)   X(sf)    \
    X(stats)  X(rm)    X(fv)    \
    X(pwd)    X(find)  X(showfv) X(cp) \
    X(fs)     X(fc)                   \

// Global jump tables
extern std::string commands[64];
extern void (*cmds_defs[64])(File*&, const std::vector<std::string>&);

// Forward declarations for handlers
#define X(name) void handle_##name(File*& current, const std::vector<std::string>& args);
COMMAND_LIST
#undef X

void build_commands();
int get_idx(std::string s);
void compile_commands(std::string s, File*& current, std::vector<std::string> args);
void pwd(File* current);
File* find_child(File* current, const std::string& name);

#endif
