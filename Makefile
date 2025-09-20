# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g
GTEST_FLAGS = -lgtest -lgtest_main -pthread
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
	$(CXX) $(OBJECTS) -o $(TARGET)

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test executable
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(OBJDIR)/main.o, $(OBJECTS))
	$(CXX) $^ $(GTEST_FLAGS) -o $(TEST_TARGET)

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