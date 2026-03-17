#ifndef COMMANDS_H
#define COMMANDS_H

#include "FileSystem.h"
#include <string>
#include <vector>

// 1. The List (Ensure every line has two arguments)
#define COMMAND_LIST \
    X(help, "Displays this help menu") \
    X(ls, "Lists files and directories") \
    X(mkdir, "Creates a new directory") \
    X(cd, "Changes current directory") \
    X(save, "Saves filesystem state") \
    X(load, "Loads filesystem state") \
    X(tree, "Shows directory tree") \
    X(wtf, "Create/Edit a file (q to stop)") \
    X(sf, "Show file content") \
    X(stats, "Show system statistics") \
    X(rm, "Remove file or directory") \
    X(fv, "Toggle favorite status") \
    X(pwd, "Print working directory") \
    X(find, "Search for node by name") \
    X(showfv, "List all favorites") \
    X(cp, "Copy a node") \
    X(fs, "Show recursive node size") \
    X(fc, "Search files by content")   \
	X(exc, "excucte the .orbit file") \
    X(history,"shows the history of user inputs.")
    

// Global jump tables
extern std::string commands[64];
extern std::string command_descs[64]; // Add this if not there
extern void (*cmds_defs[64])(File*&, const std::vector<std::string>&);

// 2. The Forward Declaration Fix
// Note: We add 'desc' here so the macro can handle the input from the list
#define X(name, desc) void handle_##name(File*& current, const std::vector<std::string>& args);
COMMAND_LIST
#undef X

void build_commands();
int get_idx(std::string s);
void compile_commands(std::string s, File*& current, std::vector<std::string> args);
void pwd(File* current);
File* find_child(File* current, const std::string& name);

#endif
