#include "../Headers/Commands.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

using namespace std;

// --- Helper Functions ---

/**
 * Finds a child node by its name within a given parent directory.
 * Returns a raw pointer for non-owning access.
 */
File* find_child(File* current, const string& name) {
    if (!current) return nullptr;
    for (auto const& child : current->children) {
        if (child->name == name) return child.get();
    }
    return nullptr;
}

/**
 * Generates a hash index for the command jump table.
 */
int get_idx(string s) {
    if (s.empty()) return -1;
    return (unsigned(s[0] * 31 + s[s.size() - 1])) % 64;
}

/**
 * Maps command strings to their respective handler functions using X-Macros.
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
 * Executes the command by looking it up in the jump table.
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
 * Recursively prints the current directory path from root to current.
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

/**
 * Displays all available commands in OrbitOS.
 */
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

/**
 * Lists contents of the current directory.
 * [*] denotes a favorite (protected) file.
 */
void handle_ls(File*& current, const vector<string>& args) {
    for (auto const& child : current->children) {
        string favPrefix = child->isFav ? "[*]" : "";
        cout << favPrefix << child->name << (child->isDir ? "/" : "") << "  ";
    }
    cout << endl;
}

/**
 * Advanced mkdir: Supports path creation (e.g., mkdir a/b/c).
 * If a directory in the path doesn't exist, it creates it.
 */
void handle_mkdir(File*& current, const vector<string>& args) {
    if (args.empty()) return;

    File* temp = current; 
    for (const string& part : args) {
        File* next = find_child(temp, part);
        
        if (next == nullptr) {
            // Determine if it's a file or directory based on presence of '.'
            bool isDir = true;
            for(auto c : part) if(c == '.') isDir = false;

            auto newNode = make_unique<File>(part, isDir, false, temp);
            File* newNodePtr = newNode.get();
            temp->children.push_back(move(newNode));
            temp = newNodePtr; // Descend into the new directory
        } else {
            if (!next->isDir) {
                cout << "mkdir: '" << part << "': File exists" << endl;
                return;
            }
            temp = next;
        }
    }
}

/**
 * Advanced cd: Navigates through a multi-part path provided by the parser.
 */
void handle_cd(File*& current, const vector<string>& args) {
    if (args.empty()) return;

    File* target = current;
    for (const string& part : args) {
        if (part == "root") {
            while (target->parent) target = target->parent;
        } else if (part == "..") {
            if (target->parent) target = target->parent;
        } else {
            File* next = find_child(target, part);
            if (next && next->isDir) {
                target = next;
            } else {
                cout << "OrbitOS: cd: " << part << ": No such directory" << endl;
                return;
            }
        }
    }
    current = target;
}

/**
 * Toggles the favorite status of a file. Favorites are protected from deletion.
 */
void handle_fv(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* child = find_child(current, args[0]);
    if (child) {
        child->isFav = !child->isFav;
        cout << "File '" << child->name << "' favorite: " << (child->isFav ? "ON" : "OFF") << endl;
    } else {
        cout << "fv: " << args[0] << ": No such file" << endl;
    }
}

/**
 * Recursively finds a file by name starting from root.
 */
void handle_find(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    bool isAuth = (args.size() > 1 && args.back() == "authorized");

    if (!isAuth && current->parent != nullptr) {
        cout << "Error: find must start from root." << endl;
        return;
    }

    static bool foundAny = false;
    if (!isAuth) foundAny = false;

    if (current->name == args[0]) {
        cout << "Found: "; pwd(current); cout << endl;
        foundAny = true;
    }

    vector<string> authArgs = args;
    if (!isAuth) authArgs.push_back("authorized");

    for (auto const& child : current->children) {
        File* childPtr = child.get();
        handle_find(childPtr, authArgs);
    }

    if (!isAuth && !foundAny) cout << "File not found." << endl;
}

/**
 * Advanced rm: Supports -r for recursive directory deletion and path navigation.
 */
void handle_rm(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    
    bool recursive = (args[0] == "-r");
    size_t name_idx = recursive ? 1 : 0;
    if (args.size() <= name_idx) return;

    File* target_dir = current;
    // Navigate to the parent directory of the target item
    for (size_t i = name_idx; i < args.size() - 1; i++) {
        target_dir = find_child(target_dir, args[i]);
        if (!target_dir || !target_dir->isDir) {
            cout << "rm: path not found" << endl;
            return;
        }
    }

    string target_name = args.back();
    auto it = target_dir->children.begin();
    while (it != target_dir->children.end()) {
        if ((*it)->name == target_name) {
            if ((*it)->isFav) { cout << "rm: Guarded file." << endl; return; }
            if ((*it)->isDir && !recursive) { cout << "rm: Is a directory (use -r)." << endl; return; }
            target_dir->children.erase(it);
            return;
        }
        it++;
    }
    cout << "rm: " << target_name << ": No such file" << endl;
}

/**
 * Renders a visual tree structure of the file system.
 */
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

/**
 * Write To File (wtf): Records user input into a file's content buffer.
 */
void handle_wtf(File*& current, const vector<string>& args) {
    if (args.empty()) return;

    File* target = find_child(current, args[0]);

    if (target != nullptr && target->isDir) {
        cout << "wtf: target is a directory." << endl;
        return;
    }

    if (target != nullptr && !target->Content.empty()) {
        cout << "Overwrite? (y/n): ";
        char choice; cin >> choice; cin.ignore(1000, '\n');
        if (choice == 'n') return;
    } 
    
    if (target == nullptr) {
        current->children.push_back(make_unique<File>(args[0], false, false, current));
        target = current->children.back().get();
    }

    string buffer = "";
    char c;
    cout << "Recording... (Type 'q' to stop)" << endl;
    while (cin.get(c) && c != 'q') buffer += c;
    cin.ignore(1000, '\n'); 
    
    target->Content = buffer;
    cout << "File Saved." << endl;
}

/**
 * Show File (sf): Prints the content of a file.
 */
void handle_sf(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* target = current;
    
    for (size_t i = 0; i < args.size() - 1; i++) {
        target = find_child(target, args[i]);
        if (!target) { cout << "Path not found." << endl; return; }
    }

    File* file = find_child(target, args.back());
    if (file && !file->isDir) {
        cout << "--- Content of " << file->name << " ---" << endl;
        cout << file->Content << endl;
        cout << "-----------------------" << endl;
    } else {
        cout << "sf: file not found." << endl;
    }
}

// --- Stats and Global Favorites Logic ---

int amount_of_files = 0, deepest_file = 0, current_depth = 0;

void stats_recursive(File* current) {
    amount_of_files++;
    if (current_depth > deepest_file) deepest_file = current_depth;

    for (auto &child : current->children) {
        current_depth++;
        stats_recursive(child.get());
        current_depth--;
    }
}

void handle_stats(File*& current, const vector<string>& args) {
    File* root = current;
    while(root->parent) root = root->parent;

    amount_of_files = 0; deepest_file = 0; current_depth = 0;
    stats_recursive(root);

    cout << "--- OrbitOS Statistics ---" << endl;
    cout << "Total Nodes:   " << amount_of_files << endl;
    cout << "Max Depth:     " << deepest_file << endl;
}

vector<string> FavFiles;
void showfv_recursive(File *current) {
    if (current->isFav) FavFiles.push_back(current->name);
    for (auto &child : current->children) showfv_recursive(child.get());
}

void handle_showfv(File *&current, const vector<string> &args) {
    File* root = current;
    while(root->parent) root = root->parent;

    FavFiles.clear();
    showfv_recursive(root);

    cout << "Favorite Files: " << FavFiles.size() << endl;
    for (const string &s : FavFiles) cout << "- " << s << endl;
}
/**
 * Helper: Recursively clones a File node. 
 * Necessary because unique_ptr cannot be copied, only moved.
 */
unique_ptr<File> clone_node(File* source, File* new_parent) {
    // 1. Create a brand new instance in the Heap
    auto newNode = make_unique<File>(source->name, source->isDir, source->isFav, new_parent);
    newNode->Content = source->Content;

    // 2. Recursively clone all children (The Deep Copy)
    for (auto const& child : source->children) {
        newNode->children.push_back(clone_node(child.get(), newNode.get()));
    }
    return newNode;
}

/**
 * Optimized handle_cp: Matches the global command signature.
 * Uses an index offset instead of modifying the vector.
 */
void handle_cp(File*& current, const vector<string>& args) {
    if (args.size() < 2) {
        cout << "cp: missing file operand" << endl;
        return;
    }

    // 1. Check for flag without erasing
    bool is_recursive = false;
    int src_idx = 0;
    int dest_idx = 1;

    if (args[0] == "-r") {
        is_recursive = true;
        src_idx = 1;
        dest_idx = 2;
    }

    // Ensure we still have src and dest after the flag
    if (args.size() <= dest_idx) {
        cout << "cp: missing destination operand" << endl;
        return;
    }

    // 2. Locate Source
    File* src_node = find_child(current, args[src_idx]);
    if (!src_node) {
        cout << "cp: " << args[src_idx] << ": No such file" << endl;
        return;
    }

    // 3. Directory safety check
    if (src_node->isDir && !is_recursive) {
        cout << "cp: -r not specified; omitting directory '" << src_node->name << "'" << endl;
        return;
    }

    // 4. Locate Destination
    string dest_name = args[dest_idx];
    if (find_child(current, dest_name)) {
        cout << "cp: destination '" << dest_name << "' already exists" << endl;
        return;
    }

    // 5. Clone and Move
    auto cloned = clone_node(src_node, current);
    cloned->name = dest_name;
    current->children.push_back(move(cloned));

    cout << "Success: copied '" << src_node->name << "' to '" << dest_name << "'" << endl;
}
