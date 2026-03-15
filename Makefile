CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Core Engine Files (No main)
CORE_SRCS = src/Commands.cpp src/FileSystem.cpp src/orbitCommands.cpp

# --- Main OS Target ---
run:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) src/main.cpp -o build/orbit_os
	@./build/orbit_os

# --- Test Suite Target ---
test:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) tests/test.cpp -o build/fs_test
	@echo "🧪 Running Automated Tests..."
	@./build/fs_test
