


ORBIT OS | Virtual File System & Command Engine
ORBIT OS is a lightweight, high-performance Virtual File System (VFS) simulation written in C++17. Developed as part of a real-time systems research track, it focuses on efficient memory management, recursive data structures, and optimized command dispatching.

🛠 Engineering Highlights
Jump Table Dispatch: Utilizes X-Macros and a custom Hash Mapping function to achieve O(1) command lookup, mimicking low-level kernel dispatchers.

Memory Architecture: Implements a strict RAII (Resource Acquisition Is Initialization) pattern using std::unique_ptr. This ensures that deleting a directory automatically and safely deallocates all nested children, preventing memory leaks.

Recursive Navigation: Built on an N-ary Tree structure. The system handles stateful navigation (CWD) and utilizes DFS (Depth-First Search) for global file discovery.

Safety Guards: Features a Favorite Flag (isFav) system that acts as a write-protection layer, preventing the deletion of critical system files unless manually unlocked.

📂 System Architecture
The project is structured to separate the core logic from the shell interface:

FileSystem: Defines the File node and the OrbitManager which tracks the system root and current state.

Commands: Contains the logic for all shell operations and the jump-table mapping.

main.cpp: The entry point, handling the REPL (Read-Eval-Print Loop) and operator authentication.


