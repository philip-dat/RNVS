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
CMAKE_COMMAND = /snap/clion/138/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/138/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tkn/Desktop/RNVS/RNVS/Block4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/peer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/peer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/peer.dir/flags.make

CMakeFiles/peer.dir/peer.c.o: CMakeFiles/peer.dir/flags.make
CMakeFiles/peer.dir/peer.c.o: ../peer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/peer.dir/peer.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/peer.dir/peer.c.o   -c /home/tkn/Desktop/RNVS/RNVS/Block4/peer.c

CMakeFiles/peer.dir/peer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/peer.dir/peer.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/tkn/Desktop/RNVS/RNVS/Block4/peer.c > CMakeFiles/peer.dir/peer.c.i

CMakeFiles/peer.dir/peer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/peer.dir/peer.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/tkn/Desktop/RNVS/RNVS/Block4/peer.c -o CMakeFiles/peer.dir/peer.c.s

# Object files for target peer
peer_OBJECTS = \
"CMakeFiles/peer.dir/peer.c.o"

# External object files for target peer
peer_EXTERNAL_OBJECTS =

peer: CMakeFiles/peer.dir/peer.c.o
peer: CMakeFiles/peer.dir/build.make
peer: CMakeFiles/peer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable peer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/peer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/peer.dir/build: peer

.PHONY : CMakeFiles/peer.dir/build

CMakeFiles/peer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/peer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/peer.dir/clean

CMakeFiles/peer.dir/depend:
	cd /home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tkn/Desktop/RNVS/RNVS/Block4 /home/tkn/Desktop/RNVS/RNVS/Block4 /home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug /home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug /home/tkn/Desktop/RNVS/RNVS/Block4/cmake-build-debug/CMakeFiles/peer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/peer.dir/depend

