# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = "/Users/Chrissi/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/203.5784.34/CLion 2020.3 EAP.app/Contents/bin/cmake/mac/bin/cmake"

# The command to remove a file.
RM = "/Users/Chrissi/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/203.5784.34/CLion 2020.3 EAP.app/Contents/bin/cmake/mac/bin/cmake" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Block.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Block.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Block.dir/flags.make

CMakeFiles/Block.dir/client.c.o: CMakeFiles/Block.dir/flags.make
CMakeFiles/Block.dir/client.c.o: ../client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Block.dir/client.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Block.dir/client.c.o   -c /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/client.c

CMakeFiles/Block.dir/client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Block.dir/client.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/client.c > CMakeFiles/Block.dir/client.c.i

CMakeFiles/Block.dir/client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Block.dir/client.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/client.c -o CMakeFiles/Block.dir/client.c.s

# Object files for target Block
Block_OBJECTS = \
"CMakeFiles/Block.dir/client.c.o"

# External object files for target Block
Block_EXTERNAL_OBJECTS =

Block: CMakeFiles/Block.dir/client.c.o
Block: CMakeFiles/Block.dir/build.make
Block: CMakeFiles/Block.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable Block"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Block.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Block.dir/build: Block

.PHONY : CMakeFiles/Block.dir/build

CMakeFiles/Block.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Block.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Block.dir/clean

CMakeFiles/Block.dir/depend:
	cd /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2 /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2 /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug /Users/Chrissi/Documents/Bildung/Uni/2021_wise/rnvs/Block2/cmake-build-debug/CMakeFiles/Block.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Block.dir/depend

