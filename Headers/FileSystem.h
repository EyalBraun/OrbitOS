#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <memory>

class node {
public:
    bool isdir;
    std::string name;
    node* parent; 
    std::vector<std::unique_ptr<node>> children;

    node(std::string n, bool d, node* p);
};

#endif
