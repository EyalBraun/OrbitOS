#include "../Headers/Commands.h"
#include <iostream>

using namespace std;

int get_idx(string s) {
    if (s.empty()) return -1;
    // Corrected math syntax
    return (s[0] * 31 + s[s.size() - 1]) % 64;
}

void build_commands() {
    // Fill jump table
    #define X(name) cmds_defs[get_idx(#name)] = handle_##name;
    COMMAND_LIST
    #undef X
}

void compile_commands(string s, node*& current, vector<string> args) {
    int idx = get_idx(s);
    if (idx != -1 && cmds_defs[idx] != nullptr) {
        cmds_defs[idx](current, args);
    } else {
        cout << s << ": command not found" << endl;
    }
}

// Implement the "Handles" (Wrappers)
void handle_pwd(node*& current, const vector<string>& args) {
    pwd(current);
    cout << endl;
}

void handle_ls(node*& current, const vector<string>& args) {
    for (auto const& child : current->children) {
        cout << child->name << (child->isdir ? "/" : "") << "  ";
    }
    cout << endl;
}

void handle_mkdir(node*& current, const vector<string>& args) {
    if (args.empty()) return;
    current->children.push_back(make_unique<node>(args[0], true, current));
}

void handle_cd(node*& current, const vector<string>& args) {
    if (args.empty()) return;
    for (auto const& child : current->children) {
        if (child->name == args[0] && child->isdir) {
            current = child.get();
            return;
        }
    }
    cout << "Directory not found." << endl;
}

// Core Logic for PWD
void pwd(node* current) {
    if (current == nullptr) return;
    if (current->parent != nullptr) pwd(current->parent);
    cout << "/" << current->name;
}



