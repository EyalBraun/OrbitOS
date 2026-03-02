#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include "FileSystem.h" // Crucial: Compiler needs to know 'node'

#define COMMAND_LIST \
    X(pwd)           \
    X(cd)            \
    X(mkdir)         \
    X(ls)

typedef void (*CommandFunc)(node*& current, const std::vector<std::string>& args);

// Automated declaration of handle_pwd, handle_cd, etc.
#define X(name) void handle_##name(node*& current, const std::vector<std::string>& args);
COMMAND_LIST
#undef X

inline CommandFunc cmds_defs[64];
inline std::string commands[64];

void build_commands();
int get_idx(std::string s);
void compile_commands(std::string s, node*& current, std::vector<std::string> args);

// Logic Prototypes
void pwd(node* current);
void ls(node* current);

#endif
