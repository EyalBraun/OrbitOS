#include "../Headers/FileSystem.h"

using namespace std;

node::node(string n, bool d, node* p) : name(n), isdir(d), parent(p) {
    // Constructor body
}
