#include "../include/FileSystem.h"
#include "../include/Commands.h"
#include "../include/orbitCommands.h"
#include <iostream>

using namespace std;

int main() {
    build_commands();
    OrbitManager orbit;
    string owner;

    cout << "------------------------------------------" << endl;
    cout << "    ORBIT OS v1.0 | Real-Time Subsystem    " << endl;
    cout << "------------------------------------------" << endl;
    cout << "Enter Operator Name: ";
    cin >> owner; cin.ignore();

    while (true) {
        cout << "\033[1;32m" << owner << "@orbit\033[0m:";
        pwd(orbit.current);
        cout << "$ ";

        string input;
        if (!getline(cin, input) || input == "exit") break;
        if (input.empty()) continue;

        vector<string> args;
        string cmd = "", word = "";
        bool found_cmd = false;

        for (char c : input) {
            if (c != ' ') word += c;
            else if (!word.empty()) {
                if (!found_cmd) { cmd = word; found_cmd = true; }
                else args.push_back(word);
                word = "";
            }
        }
        if (!word.empty()) { if (!found_cmd) cmd = word; else args.push_back(word); }
        if (!cmd.empty()) compile_commands(cmd, orbit.current, args);
    }
    return 0;
}
