#include "../Headers/Commands.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <algorithm>

using namespace std;

// --- Globals ---
string commands[64];
void (*cmds_defs[64])(File*&, const vector<string>&) = {nullptr};

// --- Logic & Helpers ---

int get_idx(string s) {
    if (s.empty()) return -1;
    // פונקציית ה-Hash החדשה שלך
    return (unsigned((s[0] * 31) ^ (s[s.size() - 1] * 91) ^ (s.size() * 13))) % 64;
}

File* find_child(File* current, const string& name) {
    for (auto const& child : current->children) 
        if (child->name == name) return child.get();
    return nullptr;
}

void pwd_rec(File* current) {
    if (!current) return;
    if (current->parent) pwd_rec(current->parent);
    cout << (current->name == "/" ? "/" : current->name + "/");
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
    auto newNode = make_unique<File>(source->name, source->isDir, source->isFav, new_parent, source->Content);
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
    out.write((char*)&cLen, sizeof(cLen));
    out.write(node->Content.c_str(), cLen);
    out.write((char*)&cCount, sizeof(cCount));
    for (auto const& child : node->children) save_recursive(child.get(), out);
}

unique_ptr<File> load_recursive(ifstream& in, File* parent) {
    size_t nLen, cLen, cCount;
    bool d, f;
    if (!in.read((char*)&nLen, sizeof(nLen))) return nullptr;
    string name(nLen, ' '); in.read(&name[0], nLen);
    in.read((char*)&d, sizeof(d)); in.read((char*)&f, sizeof(f));
    in.read((char*)&cLen, sizeof(cLen));
    string content(cLen, ' '); if (cLen > 0) in.read(&content[0], cLen);
    auto newNode = make_unique<File>(name, d, f, parent, content);
    if (!in.read((char*)&cCount, sizeof(cCount))) return newNode;
    for (size_t i = 0; i < cCount; ++i) {
        auto child = load_recursive(in, newNode.get());
        if (child) newNode->children.push_back(move(child));
    }
    return newNode;
}

// --- Command Handlers ---

void handle_help(File*& current, const vector<string>& args) {
    cout << "OrbitOS Commands: ";
    for (int i = 0; i < 64; i++) if (!commands[i].empty()) cout << commands[i] << " ";
    cout << endl;
}

void handle_ls(File*& current, const vector<string>& args) {
    for (auto const& c : current->children) 
        cout << (c->isFav ? "[*]" : "") << c->name << (c->isDir ? "/" : "") << "  ";
    cout << endl;
}

void handle_mkdir(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* temp = current;
    for (const string& part : args) {
        File* next = find_child(temp, part);
        if (!next) {
            auto newNode = make_unique<File>(part, part.find('.') == string::npos, false, temp);
            File* ptr = newNode.get();
            temp->children.push_back(move(newNode));
            temp = ptr;
        } else temp = next;
    }
}

void handle_cd(File*& current, const vector<string>& args) {
    if (args.empty()) return;
    File* target = current;
    for (const string& part : args) {
        if (part == "root") { while (target->parent) target = target->parent; }
        else if (part == "..") { if (target->parent) target = target->parent; }
        else {
            File* next = find_child(target, part);
            if (next && next->isDir) target = next;
            else { cout << "cd: " << part << ": No such directory" << endl; return; }
        }
    }
    current = target;
}

void handle_fs(File*& current, const vector<string>& args) {
    if (args.empty()) { cout << "Error: needed a file name!" << endl; return; }
    File* target = find_child(current, args[0]);
    if (target) {
        cout << "File: " << target->name << " | Rec. Size: " << get_total_size(target) << " bytes" << endl;
    } else cout << "fs: cannot find " << args[0] << endl;
}

void handle_save(File*& current, const vector<string>& args) {
    File* root = current; while (root->parent) root = root->parent;
    ofstream out("orbit_disk.bin", ios::binary);
    if (out) { save_recursive(root, out); cout << "Saved." << endl; }
}

void handle_load(File*& current, const vector<string>& args) {
    ifstream in("orbit_disk.bin", ios::binary);
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
        current->children.push_back(make_unique<File>(args[0], false, false, current)); 
        target = current->children.back().get(); 
    }
    cout << "Recording (q to stop):" << endl;
    string buf; char c; while (cin.get(c) && c != 'q') buf += c;
    cin.ignore(1000, '\n'); target->Content = buf;
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
        if(n->isFav) cout << "[*] " << n->name << endl; 
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
        if(n->name == args[0]) { pwd_rec(n); cout << endl; } 
        for(auto &c : n->children) self(self, c.get()); 
    };
    File* r = current; while(r->parent) r = r->parent;
    rec(rec, r);
}

void handle_pwd(File*& current, const vector<string>& args) { pwd_rec(current); cout << endl; }

// --- Infrastructure ---

void build_commands() {
    #define X(name) \
        { int idx = get_idx(#name); \
          cmds_defs[idx] = handle_##name; \
          commands[idx] = #name; }
    COMMAND_LIST
    #undef X
}

void compile_commands(string s, File*& current, vector<string> args) {
    int idx = get_idx(s);
    if (idx != -1 && commands[idx] == s && cmds_defs[idx]) {
        cmds_defs[idx](current, args);
    } else {
        cout << s << ": command not found" << endl;
    }
}
void pwd(File* current) {
    pwd_rec(current);
}
