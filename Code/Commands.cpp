#include "../Headers/Commands.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

// --- Helper Functions ---

/**
 * Generates a simple hash index for the command jump table
 */
int get_idx(string s) {
    if (s.empty()) return -1;
    return (unsigned(s[0] * 31 + s[s.size() - 1])) % 64;
}

/**
 * Maps command strings to their respective handler functions using X-Macros
 */
void build_commands() {
    #define X(name) \
        int idx_##name = get_idx(#name); \
        cmds_defs[idx_##name] = handle_##name; \
        commands[idx_##name] = #name; 
    
    COMMAND_LIST
    #undef X
}

/**
 * Finds and executes the command from the jump table
 */
void compile_commands(string s, File*& current, vector<string> args) {
    int idx = get_idx(s);
    if (idx != -1 && cmds_defs[idx] != nullptr) {
        cmds_defs[idx](current, args);
    } else {
        cout << s << ": command not found" << endl;
    }
}


/**
 * Recursively prints the current directory path
 */
void pwd(File* current) {
    if (current == nullptr) return;
    if (current->parent != nullptr) pwd(current->parent);
    if (current->name == "/") {
        cout << "/";
    } else {
        cout << current->name << "/";
    }
}

// --- Command Handlers ---

void handle_help(File*& current, const vector<string>& args) {
    cout << "ORBIT OS Supported commands:" << endl;
    for (int i = 0; i < 64; i++) {
        if (!commands[i].empty()) {
            cout << " - " << commands[i] << endl;
        }
    }
}

void handle_pwd(File*& current, const vector<string>& args) {
    pwd(current);
    cout << endl;
}

void handle_ls(File*& current, const vector<string>& args) {
    for (auto const& child : current->children) {
        string favPrefix = child->isFav ? "[*]" : "";
        cout << favPrefix << child->name << (child->isDir ? "/" : "") << "  ";
    }
    cout << endl;
}

void handle_mkdir(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    bool isDir = true;
    for(auto c : args[0]) if(c == '.') isDir = false;
    // Creates a new File object: Name, isDir, isFav (false), Parent
    current->children.push_back(make_unique<File>(args[0], isDir, false, current));
}

void handle_cd(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    
    // Quick jump to root
    if(args[0] == "root"){
        while(current->parent != nullptr) current = current->parent;
        return;
    }
    
    // Move to parent directory
    if (args[0] == ".." && current->parent != nullptr) {
        current = current->parent;
        return;
    }
    
    // Navigate to child directory
    for (auto const& child : current->children) {
        if (child->name == args[0] && child->isDir) {
            current = child.get();
            return;
        }
    }
    cout << "Directory not found." << endl;
}

void handle_fv(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    for (auto& child : current->children) {
        if (child->name == args[0]) {
            child->isFav = !child->isFav;
            cout << "File '" << child->name << "' favorite status: " << (child->isFav ? "ON" : "OFF") << endl;
            return;
        }
    }
    cout << "fv: " << args[0] << ": No such file" << endl;
}

void handle_find(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    bool isAuthorized = (args.size() > 1 && args.back() == "authorized");

    if (!isAuthorized && current->parent != nullptr) {
        cout << "Error: find must start from root (type 'cd root')" << endl;
        return;
    }

    static bool foundAny = false; 
    if (!isAuthorized) foundAny = false;

    if (current->name == args[0]) {
        cout << "Found: ";
        pwd(current);
        cout << endl;
        foundAny = true; 
    }

    vector<string> authorizedArgs = args; 
    if (!isAuthorized) authorizedArgs.push_back("authorized");

    // --- התיקון כאן ---
    for (auto const& child : current->children) {
        File* childPtr = child.get(); // מחלצים את המצביע למשתנה עזר (Lvalue)
        handle_find(childPtr, authorizedArgs); // עכשיו ניתן להעביר אותו כרפרנס
    }

    if (!isAuthorized && !foundAny) cout << "File not found." << endl;
}

void handle_rm(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    bool recursive = false;
    string name = (args[0][0] == '-') ? (args.size() > 1 ? args[1] : "") : args[0];
    if (args[0][0] == '-' && args[0].find('r') != string::npos) recursive = true;

    auto it = current->children.begin();
    while (it != current->children.end()) {
        if ((*it)->name == name) {
            // Check favorite guard
            if ((*it)->isFav) { cout << "rm: Guarded file." << endl; return; }
            // Check directory removal without -r flag
            if ((*it)->isDir && !recursive) { cout << "rm: Is a directory." << endl; return; }
            
            current->children.erase(it);
            return;
        }
        it++;
    }
}

void tree_recursive(File* node, string indent, bool isLast) {
    string marker = isLast ? "└── " : "├── ";
    cout << indent << marker << node->name << (node->isDir ? "/" : "") << endl;
    
    string newIndent = indent + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < node->children.size(); i++) {
        tree_recursive(node->children[i].get(), newIndent, i == node->children.size() - 1);
    }
}

void handle_tree(File*& current, const vector<string>& args) {
    cout << "." << endl; 
    for (size_t i = 0; i < current->children.size(); i++) {
        tree_recursive(current->children[i].get(), "", i == current->children.size() - 1);
    }
}


void handle_wtf(File*& current, const vector<string>& args) {
    if (args.empty()) return;

    File* target = nullptr;
    for (auto& child : current->children) {
        if (child->name == args[0]) {
            if (child->isDir) { 
                cout << "wtf: target is directory." << endl; 
                return; 
            }
            target = child.get();
            break;
        }
    }

    // Handle overwrite
    if (target != nullptr && !target->Content.empty()) {
        cout << "File exists. Overwrite? (y/n): ";
        char input; cin >> input; cin.ignore(1000, '\n');
        if (input == 'n') return;
    } 
    
    if (target == nullptr) {
        current->children.push_back(make_unique<File>(args[0], false, false, current));
        target = current->children.back().get();
    }

    char c;
    string buffer = "";
    cout << "Recording... (Type 'q' to stop)" << endl;

    while (cin.get(c)) {
        // Stop before 'q' is added to the buffer
        if (c == 'q') break; 
        buffer += c;
    }
    
    // Clear the remaining 'newline' to prevent duplicate prompt issues
    cin.ignore(1000, '\n'); 
    
    target->Content = buffer;
    cout << "\nFile Saved." << endl;
}
void handle_sf(File*& current, const vector<string>& args){
  string content;
  for(auto& child : current->children){
    if(args[0] == child->name){
      content = child->Content;
      break;
    }
  }
  cout<<content<<endl;



 }
// Global variables to track system metrics
int amount_of_files = 0;   // Counter for total nodes (files/dirs)
int deepest_file = -1;     // Tracks the maximum depth reached in the tree
int current_depth = 0;     // Current depth during recursion

/**
 * Recursive function to traverse the file system tree.
 * Updates global statistics for node count and maximum depth.
 */
void stats(File* current) {
    // Increment the total node count
    amount_of_files++;
    
    // Update deepest_file if the current path is deeper than previous records
    if (current_depth > deepest_file) {
        deepest_file = current_depth;
    }

    // Iterate through all children using raw pointers (non-owning access)
    for (auto &child : current->children) {
        current_depth++;    // Descend to the next level
        stats(child.get()); // Recursive call
        current_depth--;    // Backtrack to the parent level
    }
}

/**
 * Command handler for the 'stats' command.
 * Validates state and initiates the recursive count.
 */
void handle_stats(File*& current, const vector<string>& args) {
    // Ensuring stats are calculated from the root for a full system overview
    if (current->parent != nullptr) {
        cout << "Error: Stats can only be calculated from root. Type 'cd root' first." << endl;
        return;
    }

    // Reset global metrics before starting the calculation
    amount_of_files = 0;
    deepest_file = 0;
    current_depth = 0;

    // Execute the recursive traversal
    stats(current); 

    // Output the final results once the entire tree has been processed
    cout << "--- OrbitOS System Statistics ---" << endl;
    cout << "Total Nodes:   " << amount_of_files << endl;
    cout << "Maximum Depth: " << deepest_file << " levels" << endl;
    cout << "---------------------------------" << endl;
}

// Global vector to store names of favorite files
vector<string> FavFiles;

/**
 * Recursively traverses the tree and collects names of files 
 * marked as favorites (isFav == true).
 */
void showfv(File *current) {
    // If the current file is marked as favorite, add its name to the list
    if (current->isFav) {
        FavFiles.push_back(current->name);
    }

    // Base case: if no children, stop recursion for this branch
    if (current->children.empty()) return;

    // Recursive step: visit all children
    for (auto &child : current->children) {
        showfv(child.get());
    }
}

/**
 * Handler for the 'showfv' command.
 * Ensures the search starts from the root and displays the collected favorites.
 */
void handle_showfv(File *&current, const vector<string> &args) {
    // Security check: ensure we are at the root for a global search
    if (current->parent != nullptr) {
        cout << "Error: cannot use command showfv when not at the root directory." << endl;
        cout << "To go to the root directory use: cd root" << endl;
        return;
    }

    // CRITICAL: Clear the previous results before starting a new search
    FavFiles.clear();

    // Start the recursive search
    showfv(current);

    // Display the results
    if (FavFiles.empty()) {
        cout << "No favorite files found in the system." << endl;
    } else {
        cout << "Total favorite files found: " << FavFiles.size() << endl;
        cout << "-------------------------------" << endl;
        for (const string &s : FavFiles) {
            cout << "- " << s << endl;
        }
    }
}

  


