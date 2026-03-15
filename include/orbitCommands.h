
#ifndef ORBIT_COMMANDS_H
#define ORBIT_COMMANDS_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

#define ORB_COMMAND_LIST \
    X(ptc, "prints to console!") \
    X(gi, "Gets input from OrbitOS user!")


extern string orb_commands[64];
extern string orb_command_descs[64];
extern void (*orb_cmds_defs[64])(vector<string> args);

#define X(name, desc) void handle_##name(vector <string> args);
ORB_COMMAND_LIST
#undef X

void build_orb_commands();
vector<string> Parse_Code(string Content);
int get_orb_idx(string s);
int lookup_orb_command(const string &);
void Parse_Lines(const vector <string> &);
vector <string> Parse_Code(string);


#endif

