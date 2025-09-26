# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# GTest configuration - different for macOS vs Linux
ifeq ($(shell uname), Darwin)
	GTEST_FLAGS = -L/opt/homebrew/opt/googletest/lib -I/opt/homebrew/opt/googletest/include -lgtest -lgtest_main -pthread
else
	GTEST_FLAGS = -lgtest -lgtest_main -pthread
endif

# LLVM configuration
LLVM_CONFIG = llvm-config-15
# Filter out conflicting LLVM flags and suppress LLVM warnings
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags | sed 's/-std=c++[0-9][0-9]//g' | sed 's/-fno-exceptions//g')
ifeq ($(shell uname), Darwin)
	LLVM_CXXFLAGS += -Iinclude -I/opt/homebrew/include -I/opt/homebrew/opt/llvm@15/include -I/opt/homebrew/opt/googletest/include
else
	LLVM_CXXFLAGS += -Iinclude -I/usr/include/llvm-15
endif
CXXFLAGS += $(LLVM_CXXFLAGS) -Wno-unused-parameter -Wno-switch
LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter target)
SRCDIR = src
OBJDIR = build
TESTDIR = tests

# Find all .cpp files in src and subdirectories
SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Find runtime C files
RUNTIME_SOURCES = $(shell find $(SRCDIR)/runtime -name "*.c" 2>/dev/null || true)
RUNTIME_OBJECTS = $(RUNTIME_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Test files
TEST_SOURCES = $(shell find $(TESTDIR) -name "*.cpp")
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.cpp=$(OBJDIR)/test/%.o)

# Executable name
TARGET = forge
TEST_TARGET = test_forge

# Default target
all: $(TARGET)

# Build main executable
$(TARGET): $(OBJECTS) $(RUNTIME_OBJECTS)
	$(CXX) $(OBJECTS) $(RUNTIME_OBJECTS) -o $(TARGET) $(LDFLAGS) -g

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build runtime C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	gcc -c $< -o $@

# Build test executable
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) $(RUNTIME_OBJECTS)
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