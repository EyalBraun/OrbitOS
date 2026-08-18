# OrbitOS — Virtual File System and Command Dispatch Engine

OrbitOS is a C++17 systems-programming project that implements an in-memory virtual file system, a command-line interface and a small command-dispatch layer.

The project is intended as a learning and engineering exercise in ownership, tree data structures, command parsing, persistence and modular C++ design.

## Features

- In-memory N-ary file and directory tree.
- Ownership of child nodes through `std::unique_ptr`.
- Navigation commands such as `cd`, `pwd`, `ls` and `tree`.
- File and directory manipulation commands including `mkdir`, `rm`, `cp` and `wtf`.
- Recursive search and content search.
- Favorite/write-protection flags.
- Binary save/load support for the current file-system state.
- A command dispatcher generated from an X-macro command list.
- A small `.orb` scripting prototype.
- A diagnostic command test runner.

## Architecture

```text
CLI input
   │
   ▼
Command tokenizer
   │
   ▼
Hashed command dispatcher
   │
   ▼
File-system command handlers
   │
   ▼
N-ary File tree owned by unique_ptr
   │
   └── persistence and search helpers
```

The command dispatcher is designed to provide constant-time average lookup under its hash-table assumptions. This claim applies to command lookup; filesystem traversal and search remain dependent on the number and shape of nodes.

## Repository structure

```text
include/       Public class and command declarations
src/           Filesystem, command and CLI implementations
tests/         Diagnostic test driver and command script
docs/          Additional project documentation
Makefile       Build and test commands
```

## Build and run

On a Linux environment with a C++17 compiler:

```shell
make run
```

## Run the diagnostic suite

```shell
make test
```

The current test runner executes a scripted set of commands and reports command-level failures. It is a smoke-test suite rather than a complete unit, property-based or fuzz-testing framework.

## Current limitations

The persistence format is a simple project-specific binary representation and is not intended as a stable cross-version file format. Error handling is lightweight, and malformed input, corrupted persistence files and unusual quoting cases require additional tests.

The `.orb` scripting subsystem is an experimental prototype. Some commands and parsing paths are incomplete and should not be treated as a production scripting language.

## Roadmap

- Add unit tests for hashing, collisions, parsing and filesystem invariants.
- Add tests for corrupted and truncated save files.
- Define a versioned persistence format.
- Improve path parsing and quoted-argument handling.
- Complete the `.orb` scripting commands.
- Add sanitizers and a CI build with GCC and Clang.
- Remove generated binaries from version control and make builds reproducible from source.

## Educational focus

OrbitOS demonstrates practical C++ concepts including RAII, smart-pointer ownership, recursive data structures, serialization, command dispatch and basic test automation. It is not an operating-system kernel; it is a user-space virtual filesystem and command environment.

## License

See the repository license if present. Contributions should include a description of the tested behavior and any persistence-format implications.
