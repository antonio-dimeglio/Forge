# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude 
GTEST_FLAGS = -lgtest -lgtest_main -pthread

# LLVM configuration
LLVM_CONFIG = llvm-config-15
# Filter out conflicting LLVM flags and suppress LLVM warnings
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags | sed 's/-std=c++[0-9][0-9]//g' | sed 's/-fno-exceptions//g')
CXXFLAGS += $(LLVM_CXXFLAGS) -I/usr/include/llvm-c-15 -Wno-unused-parameter -Wno-switch
LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter)
SRCDIR = src
OBJDIR = build
TESTDIR = tests

# Find all .cpp files in src and subdirectories
SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Test files
TEST_SOURCES = $(shell find $(TESTDIR) -name "*.cpp")
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.cpp=$(OBJDIR)/test/%.o)

# Executable name
TARGET = forge
TEST_TARGET = test_forge

# Default target
all: $(TARGET)

# Build main executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) -g

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test executable
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(OBJDIR)/main.o, $(OBJECTS))
	$(CXX) $^ $(GTEST_FLAGS) $(LDFLAGS) -o $(TEST_TARGET)

# Build test object files
$(OBJDIR)/test/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run tests
run-tests: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET)

# Create build directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Show project structure
structure:
	tree -I 'build|*.o'

.PHONY: all test run run-tests clean structure