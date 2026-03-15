#include "../include/FileSystem.h"

using namespace std;

// Constructor: Content moved to the end as per the header defaults
File::File(string n, bool d, bool f, bool e, File *p, string c) 
    : name(n), isDir(d), isFav(f), isExc(e), parent(p), Content(c) {}

OrbitManager::OrbitManager() {
    // Initializing the root node. We don't store it as a unique_ptr member,
    // but its children will be unique_ptrs.
    File* root = new File("/", true, false,false, nullptr);
    current = root; 
}

OrbitManager::~OrbitManager() {
    if (!current) return;
    
    // Climb up to the very top to find the root node
    File* rootFinder = current;
    while (rootFinder->parent != nullptr) {
        rootFinder = rootFinder->parent;
    }
    // Deleting the root triggers recursive cleanup of all unique_ptr children
    delete rootFinder;
}

