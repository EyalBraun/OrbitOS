#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "../Headers/Commands.h"

using namespace std;

int main() {
    build_commands();

    auto root = make_unique<node>("root", true, nullptr);
    node* current = root.get();

    string owner;
    cout << "Owner Name: ";
    cin >> owner;
    cin.ignore();

    while (true) {
        cout << owner << "@OS:";
        pwd(current);
        cout << "$ ";

        string input;
        getline(cin, input);
        if (input == "exit") break;
        if (input.empty()) continue;

        stringstream ss(input);
        string cmd;
        ss >> cmd;
        vector<string> args;
        string a;
        while (ss >> a) args.push_back(a);

        compile_commands(cmd, current, args);
    }
    return 0;
}
