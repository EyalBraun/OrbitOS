#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include "FileSystem.h" 

#define COMMAND_LIST \
    X(pwd)           \
    X(cd)            \
    X(mkdir)         \
    X(ls)            \
    X(help)          \
    X(rm)            \
    X(fv)            \
    X(find)          \
    X(tree)          \
    X(wtf)           \
    X(sf)            \
    X(stats)         \
    X(showfv)        \

// Update typedef to use File*
typedef void (*CommandFunc)(File*& current, const std::vector<std::string>& args);

#define X(name) void handle_##name(File*& current, const std::vector<std::string>& args);
COMMAND_LIST
#undef X

inline CommandFunc cmds_defs[64];
inline std::string commands[64]; 

void build_commands();
void compile_commands(std::string s, File*& current, std::vector<std::string> args);
void pwd(File* current);

#endif
