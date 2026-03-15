#include "../include/orbitCommands.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// --- Global Arrays for Command Mapping ---
string orb_commands[64];
string orb_command_descs[64];
void (*orb_cmds_defs[64])(vector<string> args) = {nullptr};

// --- Hashing & Lookup Logic ---

// Hashing algorithm to map strings to an index (0-63)
int get_orb_idx(string s) {
    if (s.empty()) return -1;
    return (unsigned((s[0] * 31) ^ (s[s.size() - 1] * 91) ^ (s.size() * 13))) % 64;
}

// Safely lookup a command using the linear probe chain
int lookup_orb_command(const string& cmd) {
    int start_idx = get_orb_idx(cmd);
    if (start_idx == -1) return -1;

    int idx = start_idx;
    do {
        if (orb_commands[idx] == cmd) return idx; // Found exact match
        if (orb_commands[idx].empty()) return -1; // Hit an empty slot
        idx = (idx + 1) % 64;                     // Follow the probe chain
    } while (idx != start_idx);

    return -1; 
}

// --- Command Implementations ---

void handle_ptc(vector<string> args) {
    if (args.empty()) {
        cout << "OrbitOS Error: needed something to write to console!" << endl;
        return;
    }
    
    for (const string& arg : args) {
        cout << arg << " ";
    }
    cout << endl;
}

void handle_gi(vector<string> args) {
    cout << "OrbitOS: handle_gi is not built yet!" << endl;
}

// --- Initialization ---

void build_orb_commands() {
    // X-Macro expansion to populate the function pointers and metadata
    #define X(name, desc) \
        { \
            int idx = get_orb_idx(#name); \
            while (!orb_commands[idx].empty()) idx = (idx + 1) % 64; \
            orb_cmds_defs[idx] = handle_##name; \
            orb_commands[idx] = #name; \
            orb_command_descs[idx] = desc; \
        }
    
    ORB_COMMAND_LIST
    
    #undef X
}

// --- Parsing Logic ---

// Splits raw content into executable lines based on semicolons
vector<string> Parse_Code(string Content) {
    vector<string> Code;
    string Line = "";
    for (char c : Content) {
        // Skip carriage returns and newlines so they don't break the hash lookup
        if (c == '\n' || c == '\r') continue;

        if (c != ';') {
            Line += c;
        } else {
            if (!Line.empty()) {
                Code.push_back(Line);
            }
            Line = "";
        }
    }
    if (!Line.empty()) {
        Code.push_back(Line);
    }
    return Code;
}

// State machine to isolate commands and their quoted arguments
void Parse_Lines(const vector<string>& Lines) {
    for (size_t i = 0; i < Lines.size(); i++) {
        string cmd = "";
        vector<string> args;
        string current_arg = "";

        bool in_quotes = false;
        bool found_cmd = false;
        bool syntax_error = false;

        const string& line = Lines[i];

        for (size_t j = 0; j < line.size(); j++) {
            char c = line[j];

            // Safety check: Stop if a quote opens before a command exists
            if (c == '"' && cmd.empty()) {
                cout << "OrbitOS Error: Must use a command before opening a quote!" << endl;
                syntax_error = true;
                break; 
            }

            // Phase 1: Read the command name
            if (!found_cmd) {
                if (c == ' ') {
                    if (!cmd.empty()) found_cmd = true; 
                } else if (c == '"') {
                    found_cmd = true; 
                    in_quotes = true;
                } else {
                    cmd += c;
                }
                continue;
            }

            // Phase 2: Read the arguments inside quotes
            if (c == '"') {
                in_quotes = !in_quotes; 
                if (!in_quotes) {       
                    args.push_back(current_arg);
                    current_arg = "";   
                }
            } else if (in_quotes) {
                current_arg += c;
            }
        }

        // Phase 3: Execute the mapped function pointer
        if (!syntax_error && !cmd.empty()) {
            int target_idx = lookup_orb_command(cmd);
            
            if (target_idx != -1 && orb_cmds_defs[target_idx] != nullptr) {
                orb_cmds_defs[target_idx](args); 
            } else {
                cout << "OrbitOS Error: command not found -> " << cmd << endl;
            }
        }
    }
}
