#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <memory>

class File {
public: // (or if you are using 'struct File {', that works too)
    std::string name;
    bool isDir;
    bool isFav;
    bool isExc; // <-- Added the executable flag
    File* parent;
    std::string Content;
    std::vector<std::unique_ptr<File>> children;

    // <-- Updated the constructor to take the 4th boolean (bool e)
    File(std::string n, bool d, bool f, bool e, File *p = nullptr, std::string c = "");
};

class OrbitManager {
public:
    File *current;
    OrbitManager();
    ~OrbitManager(); 
};

#endif
