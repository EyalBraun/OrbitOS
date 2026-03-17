#include "../include/Commands.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <algorithm>
#include <limits>
#include <sstream>
#include "../include/orbitCommands.h"

using namespace std;

// --- Globals ---
string commands[64];
string command_descs[64];
void (*cmds_defs[64])(File*&, const vector<string>&) = {nullptr};
extern vector <string> cmdHistory;
vector<string> PATHS; 

// --- Logic & Helpers ---

string get_path_string(File* current) {
    if (!current) return "";
    if (current->parent == nullptr) return "/";

    string full_path = "";
    File* temp = current;
    while (temp != nullptr && temp->parent != nullptr) {
        full_path = "/" + temp->name + full_path;
        temp = temp->parent;
    }
    return full_path;
}

int get_idx(string s) {
    if (s.empty()) return -1;
    return (unsigned((s[0] * 31) ^ (s[s.size() - 1] * 91) ^ (s.size() * 13))) % 64;
}

File* find_child(File* current, const string& name) {
    for (auto const& child : current->children) 
        if (child->name == name) return child.get();
    return nullptr;
}

void pwd_rec(File* current) {
    if (!current) return;

    // 1. Recurse up to the root first
    if (current->parent) {
        pwd_rec(current->parent);
    }

    // 2. Print the current name
    if (current->name == "/") {
        cout << "/"; 
    } else {
        // Only add the trailing slash if it's not the root
        cout << current->name << "/";
    }
}
void handle_history(File*& current, const std::vector<std::string>& args) {
    if (cmdHistory.empty()) return;

    int count = 1;
    std::string last = cmdHistory[0];

    // Start loop from 1
    for (size_t i = 1; i < cmdHistory.size(); i++) {
        if (cmdHistory[i] == last) {
            count++;
        } else {
            // Print the 'last' command and its count
            if (count > 1) cout << last << " [X" << count << "]" << endl;
            else cout << last << endl;
            
            // Reset for the new command
            last = cmdHistory[i];
            count = 1;
        }
    }

    // CRITICAL: Print the very last group after the loop ends
    if (count > 1) cout << last << " [X" << count << "]" << endl;
    else cout << last << endl;
}

	



size_t get_total_size(File* node) {
    if (!node) return 0;
    size_t total = node->Content.size();
    for (auto const& child : node->children) {
        total += get_total_size(child.get());
    }
    return total;
}

unique_ptr<File> clone_node(File* source, File* new_parent) {
    // Added source->isExc to the clone constructor
    auto newNode = make_unique<File>(source->name, source->isDir, source->isFav, source->isExc, new_parent, source->Content);
    for (auto const& child : source->children) 
        newNode->children.push_back(clone_node(child.get(), newNode.get()));
    return newNode;
}

// --- Persistence ---

void save_recursive(File* node, ofstream& out) {
    if (!node) return;
    size_t nLen = node->name.size(), cLen = node->Content.size(), cCount = node->children.size();
    out.write((char*)&nLen, sizeof(nLen));
    out.write(node->name.c_str(), nLen);
    out.write((char*)&node->isDir, sizeof(node->isDir));
    out.write((char*)&node->isFav, sizeof(node->isFav));
    out.write((char*)&node->isExc, sizeof(node->isExc)); // Persistence for executable flag
    out.write((char*)&cLen, sizeof(cLen));
    out.write(node->Content.c_str(), cLen);
    out.write((char*)&cCount, sizeof(cCount));
    for (auto const& child : node->children) save_recursive(child.get(), out);
}

unique_ptr<File> load_recursive(ifstream& in, File* parent) {
    size_t nLen, cLen, cCount;
    bool d, f, e; // Added 'e' for isExc
    if (!in.read((char*)&nLen, sizeof(nLen))) return nullptr;
    string name(nLen, ' '); in.read(&name[0], nLen);
    in.read((char*)&d, sizeof(d)); 
    in.read((char*)&f, sizeof(f));
    in.read((char*)&e, sizeof(e)); // Load executable flag
    in.read((char*)&cLen, sizeof(cLen));
    string content(cLen, ' '); if (cLen > 0) in.read(&content[0], cLen);
    auto newNode = make_unique<File>(name, d, f, e, parent, content);
    if (!in.read((char*)&cCount, sizeof(cCount))) return newNode;
    for (size_t i = 0; i < cCount; ++i) {
        auto child = load_recursive(in, newNode.get());
        if (child) newNode->children.push_back(move(child));
    }
    return newNode;
}

// --- Command Handlers ---

void handle_help(File*& current, const vector<string>& args) {
    cout << "\n--- OrbitOS Command Reference ---" << endl;
    struct HelpItem { string n; string d; };
    vector<HelpItem> list;
    for (int i = 0; i < 64; i++) {
        if (!commands[i].empty()) list.push_back({commands[i], command_descs[i]});
    }
    sort(list.begin(), list.end(), [](const HelpItem& a, const HelpItem& b) { return a.n < b.n; });
    for (const auto& item : list) {
        string pad(10 - item.n.length(), ' ');
        cout << "  " << item.n << pad << "| " << item.d << endl;
    }
}

void handle_ls(File*& current, const vector<string>& args) {
    for (auto const& c : current->children) 
        cout << (c->isFav ? "[*]" : "") << c->name << (c->isDir ? "/" : "") << "  ";
    cout << endl;
}

void handle_mkdir(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    for (const string& full_arg : args) {
        File* temp = current;
        stringstream ss(full_arg);
        string part;
        while (getline(ss, part, '/')) {
            if (part.empty()) continue;
            File* next = find_child(temp, part);
            if (!next) {
                // Directories are created with isExc = false
                //find if exucutable :
                bool started = false;
                string typeOfFile = "";
                int dot_index = -1;
                for(size_t i  = 0;i<part.size();i++){
                  char c = part[i];
                  if(c == '.'){
                  started = true;
                  dot_index = i;
            

          }
                if(started) typeOfFile+=c;
              
        }
                auto newNode = make_unique<File>(part, !(started &&dot_index > 0) , false,typeOfFile == ".orb",  temp);
                File* ptr = newNode.get();
                temp->children.push_back(move(newNode));
                temp = ptr;
            } else temp = next;
        }
    }
}

void handle_exc(File*& current, const vector<string>& args) {
  
  bool is_e = false;
  File *exucutable = nullptr;
  for(auto &child : current -> children){
    if(child->name == args[0]){
      is_e = child->isExc;
      exucutable = child.get();
                    
    }
  }
  if(!is_e && exucutable) cout<<"File is not exucutable , needed the ending of .orb"<<endl;
  if(!exucutable) cout<<"couldnt find file"<<endl;
  else if (is_e && exucutable){
    cout<<"exucuting >>>>>>> "<<endl;
    Parse_Lines(Parse_Code(exucutable->Content));

    

  }
    
   
    
}
void handle_cd(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* target = current;
    for (const string& full_arg : args) {
        stringstream ss(full_arg);
        string part;
        while (getline(ss, part, '/')) {
            if (part.empty()) continue;
            if (part == "root") { while (target->parent) target = target->parent; }
            else if (part == "..") { if (target->parent) target = target->parent; }
            else {
                File* next = find_child(target, part);
                if (next && next->isDir) target = next;
                else { cout << "cd: " << part << ": No such directory" << endl; return; }
            }
        }
    }
    current = target;
}

void handle_fs(File*& current, const vector<string>& args) {
    if (args.empty()) { cout << "Error: file name required!" << endl; return; }
    File* target = find_child(current, args[0]);
    if (target) {
        cout << "File: " << target->name << " | Rec. Size: " << get_total_size(target) << " bytes" << endl;
    } else cout << "fs: cannot find " << args[0] << endl;
}

void handle_save(File*& current, const vector<string>& args) {
    File* root = current; while (root->parent) root = root->parent;
    ofstream out("data/orbit_data.bin", ios::binary);
    if (out) { save_recursive(root, out); cout << "Saved." << endl; }
}

void handle_load(File*& current, const vector<string>& args) {
    ifstream in("data/orbit_data.bin", ios::binary);
    if (!in) return;
    File* root = current; while (root->parent) root = root->parent;
    root->children.clear();
    auto fileRoot = load_recursive(in, nullptr);
    if (fileRoot) {
        for (auto& child : fileRoot->children) {
            child->parent = root;
            root->children.push_back(move(child));
        }
        current = root; cout << "Restored." << endl;
    }
}

void handle_tree(File*& current, const vector<string>& args) {
    auto rec = [](auto self, File* n, string ind, bool last) -> void {
        cout << ind << (last ? "└── " : "├── ") << n->name << (n->isDir ? "/" : "") << endl;
        for (size_t i = 0; i < n->children.size(); i++)
            self(self, n->children[i].get(), ind + (last ? "    " : "│   "), i == n->children.size()-1);
    };
    rec(rec, current, "", true);
}

void handle_wtf(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* target = find_child(current, args[0]);
    if (!target) { 
        // New files are created with isExc = false
        bool dot_start = false;
        string FileType = "";
        for(auto c : args[0]){
      if(c == '.') dot_start = true;
      if(dot_start) FileType += c;

    }
        current->children.push_back(make_unique<File>(args[0], false, false, FileType == ".orb", current)); 
        target = current->children.back().get(); 
    }
    cout << "Recording (q to stop):" << endl;
    string buf; char c; while (cin.get(c) && c != 'q') buf += c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); target->Content = buf;
}

void handle_sf(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* f = find_child(current, args[0]);
    if (f) cout << f->Content << endl;
}

void handle_rm(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    auto it = find_if(current->children.begin(), current->children.end(), 
        [&](const unique_ptr<File>& f){ return f->name == args.back(); });
    if (it != current->children.end() && !(*it)->isFav) current->children.erase(it);
}

void handle_stats(File*& current, const vector<string>& args) {
    int count = 0;
    auto rec = [&](auto self, File* n) -> void { count++; for(auto &c : n->children) self(self, c.get()); };
    File* r = current; while(r->parent) r = r->parent;
    rec(rec, r); cout << "Total Nodes: " << count << endl;
}

void handle_fv(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* c = find_child(current, args[0]);
    if (c) c->isFav = !c->isFav;
}

void handle_showfv(File*& current, const vector<string>& args) {
    auto rec = [&](auto self, File* n) -> void { 
        if(n->isFav) cout << "[*] " << n->name << " (Path: " << get_path_string(n) << ")" << endl; 
        for(auto &c : n->children) self(self, c.get()); 
    };
    File* r = current; while(r->parent) r = r->parent;
    rec(rec, r);
}

void handle_cp(File*& current, const vector<string>& args) {
    if (args.size() < 2) return;
    File* src = find_child(current, args[0]);
    if (src) {
        auto cl = clone_node(src, current);
        cl->name = args[1];
        current->children.push_back(move(cl));
    }
}

void handle_find(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    auto rec = [&](auto self, File* n) -> void { 
        if(n->name == args[0]) { cout << "Found: " << get_path_string(n) << endl; } 
        for(auto &c : n->children) self(self, c.get()); 
    };
    File* r = current; while(r->parent) r = r->parent;
    rec(rec, r);
}

void fc_rec(File* node, const string& content) {
    if (node == nullptr) return;
    if (!node->isDir && node->Content == content) {
        PATHS.push_back(get_path_string(node));
    }
    for (auto const& child : node->children) {
        fc_rec(child.get(), content);
    }
}

void handle_fc(File*& current, const vector<string>& args) {
    cout << "Recording content to search (q to stop):" << endl;
    string buf; char c; 
    while (cin.get(c) && c != 'q') buf += c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if(buf.empty()) return;

    PATHS.clear(); 
    File* root = current;
    while(root->parent) root = root->parent; 

    fc_rec(root, buf);

    if (PATHS.empty()) {
        cout << "No matching files found." << endl;
    } else {
        cout << "Found content in " << PATHS.size() << " files:" << endl;
        for (const auto& p : PATHS) cout << " -> " << p << endl;
    }
}

void handle_pwd(File*& current, const vector<string>& args) { pwd_rec(current); cout << endl; }

// --- Infrastructure ---

void build_commands() {
    #define X(name, desc) \
        { \
            int idx = get_idx(#name); \
            while (!commands[idx].empty()) idx = (idx + 1) % 64; \
            cmds_defs[idx] = handle_##name; \
            commands[idx] = #name; \
            command_descs[idx] = desc; \
        }
    COMMAND_LIST
    #undef X
}

void compile_commands(string s, File*& current, vector<string> args) {
    int idx = get_idx(s);
    int start_idx = idx;

    while (!commands[idx].empty()) {
        if (commands[idx] == s) {
            if (cmds_defs[idx]) {
                cmds_defs[idx](current, args);
                return;
            }
        }
        idx = (idx + 1) % 64;
        if (idx == start_idx) break; 
    }
    cout << s << ": command not found" << endl;
}

void pwd(File* current) {
    pwd_rec(current);
}
