#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../Headers/FileSystem.h" 
#include "../Headers/Commands.h"

using namespace std;

int main() {
    build_commands();
    
    OrbitManager orbit; 
    string owner;
    
    cout << "------------------------------------------" << endl;
    cout << "   ORBIT OS v1.0 | Real-Time Subsystem    " << endl;
    cout << "------------------------------------------" << endl;
    
    cout << "Enter Operator Name: ";
    cin >> owner;
    cin.ignore(); 

    while (true) {
        // Prompt עם צבע תכלת לנתיב
        cout << "\033[1;32m" << owner << "@orbit\033[0m:";
        pwd(orbit.current);
        cout << "$ ";

        string input;
        if (!getline(cin, input)) break;
        if (input == "exit") break;
        if (input.empty()) continue;

        vector<string> args;
        string cmd = "";
        string word = "";
        bool found_cmd = false;

        for (int i = 0; i < input.size(); i++) {
            if (input[i] != ' ' && input[i] != '/') {
                word += input[i];
            } else if (!word.empty()) {
                if (!found_cmd) {
                    cmd = word;
                    found_cmd = true;
                } else {
                    args.push_back(word);
                }
                word = "";
            }
        }
        
        if (!word.empty()) {
            if (!found_cmd) cmd = word;
            else args.push_back(word);
        }

        if (!cmd.empty()) {
        
            
          compile_commands(cmd, orbit.current, args);
        }
    }

    cout << "Shutting down ORBIT OS..." << endl;
    return 0;
}
