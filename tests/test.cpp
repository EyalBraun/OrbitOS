#include "../include/Commands.h"
#include "../include/orbitCommands.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

int main() {
    build_commands();
    build_orb_commands();
    
    auto root = make_unique<File>("/", true, false, false, nullptr, "");
    File* current = root.get();

    ifstream file("tests/test.txt");
    if (!file.is_open()) {
        cerr << "[FATAL] Could not find tests/test.txt" << endl;
        return 1;
    }

    // --- Redirection Setup ---
    streambuf* terminal_cout = cout.rdbuf(); 
    stringstream silent_buffer;            
    // -------------------------

    string line;
    int line_num = 0;
    int fail_count = 0;

    cout << "--- OrbitOS Automated Diagnostic Report ---" << endl;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; 
        line_num++;

        stringstream ss(line);
        string cmd;
        ss >> cmd;
        vector<string> args;
        string arg;
        while (ss >> arg) args.push_back(arg);

        // 1. Redirect output to the silent buffer
        cout.rdbuf(silent_buffer.rdbuf());

        // 2. Execute
        compile_commands(cmd, current, args);

        // 3. Switch back to terminal to check results
        string output = silent_buffer.str();
        cout.rdbuf(terminal_cout);

        // 4. Logic: If "Error" or "not found" is in the output, showcase it
        if (output.find("Error") != string::npos || output.find("not found") != string::npos) {
            cout << "[FAIL] Line " << line_num << ": " << line << endl;
            cout << "      Reason: " << output; // Show the hidden error message
            fail_count++;
        }

        // Clear buffer for the next loop
        silent_buffer.str("");
        silent_buffer.clear();
    }

    cout << "-------------------------------------------" << endl;
    if (fail_count == 0) {
        cout << ">> Result: [SUCCESS] All commands executed perfectly." << endl;
    } else {
        cout << ">> Result: [FAILED] " << fail_count << " issues detected." << endl;
    }

    return (fail_count == 0) ? 0 : 1;
}
