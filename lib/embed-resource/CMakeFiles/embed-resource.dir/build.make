# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wallace/Desktop/nano/nanogui-test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wallace/Desktop/nano/nanogui-test

# Include any dependencies generated for this target.
include lib/embed-resource/CMakeFiles/embed-resource.dir/depend.make

# Include the progress variables for this target.
include lib/embed-resource/CMakeFiles/embed-resource.dir/progress.make

# Include the compile flags for this target's objects.
include lib/embed-resource/CMakeFiles/embed-resource.dir/flags.make

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o: lib/embed-resource/CMakeFiles/embed-resource.dir/flags.make
lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o: lib/embed-resource/embedresource.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wallace/Desktop/nano/nanogui-test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o"
	cd /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/embed-resource.dir/embedresource.cpp.o -c /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource/embedresource.cpp

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/embed-resource.dir/embedresource.cpp.i"
	cd /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource/embedresource.cpp > CMakeFiles/embed-resource.dir/embedresource.cpp.i

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/embed-resource.dir/embedresource.cpp.s"
	cd /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource/embedresource.cpp -o CMakeFiles/embed-resource.dir/embedresource.cpp.s

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.requires:

.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.requires

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.provides: lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.requires
	$(MAKE) -f lib/embed-resource/CMakeFiles/embed-resource.dir/build.make lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.provides.build
.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.provides

lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.provides.build: lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o


# Object files for target embed-resource
embed__resource_OBJECTS = \
"CMakeFiles/embed-resource.dir/embedresource.cpp.o"

# External object files for target embed-resource
embed__resource_EXTERNAL_OBJECTS =

lib/embed-resource/embed-resource: lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o
lib/embed-resource/embed-resource: lib/embed-resource/CMakeFiles/embed-resource.dir/build.make
lib/embed-resource/embed-resource: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
lib/embed-resource/embed-resource: /usr/lib/x86_64-linux-gnu/libboost_system.so
lib/embed-resource/embed-resource: lib/embed-resource/CMakeFiles/embed-resource.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wallace/Desktop/nano/nanogui-test/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable embed-resource"
	cd /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/embed-resource.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/embed-resource/CMakeFiles/embed-resource.dir/build: lib/embed-resource/embed-resource

.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/build

lib/embed-resource/CMakeFiles/embed-resource.dir/requires: lib/embed-resource/CMakeFiles/embed-resource.dir/embedresource.cpp.o.requires

.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/requires

lib/embed-resource/CMakeFiles/embed-resource.dir/clean:
	cd /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource && $(CMAKE_COMMAND) -P CMakeFiles/embed-resource.dir/cmake_clean.cmake
.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/clean

lib/embed-resource/CMakeFiles/embed-resource.dir/depend:
	cd /home/wallace/Desktop/nano/nanogui-test && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wallace/Desktop/nano/nanogui-test /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource /home/wallace/Desktop/nano/nanogui-test /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource /home/wallace/Desktop/nano/nanogui-test/lib/embed-resource/CMakeFiles/embed-resource.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/embed-resource/CMakeFiles/embed-resource.dir/depend

