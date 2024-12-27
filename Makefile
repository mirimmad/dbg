# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Directories
INCLUDE_DIRS = -Iinclude/ -Isrc/include
SRC_DIRS = src
TOOL_DIRS = tools
OBJ_DIR = obj


# Source files and object files
SRC_FILES = $(wildcard $(SRC_DIRS)/*.cpp) $(wildcard $(TOOL_DIRS)/*.cpp)
OBJ_FILES = $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)

# Executable name
EXEC = dbg

# Target to compile the program
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -lreadline -o $(EXEC)

# Rule to compile .cpp to .o
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)  # Ensure the subdirectories of OBJ_DIR exist
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean up the object files and executable
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Phony targets
.PHONY: all clean

