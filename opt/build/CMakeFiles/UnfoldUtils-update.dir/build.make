# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake

# The command to remove a file.
RM = /cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/MAT-MINERvA/bootstrap

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build

# Utility rule file for UnfoldUtils-update.

# Include the progress variables for this target.
include CMakeFiles/UnfoldUtils-update.dir/progress.make

CMakeFiles/UnfoldUtils-update: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-update


UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-update: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-download
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Performing update step for 'UnfoldUtils'"
	cd /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/UnfoldUtils && /cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -P /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/tmp/UnfoldUtils-gitupdate.cmake

UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-download: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-gitinfo.txt
UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-download: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-mkdir
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Performing download step (git clone) for 'UnfoldUtils'"
	cd /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions && /cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -P /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/tmp/UnfoldUtils-gitclone.cmake
	cd /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions && /cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E touch /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-download

UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-mkdir:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Creating directories for 'UnfoldUtils'"
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/MAT-MINERvA/bootstrap/../../UnfoldUtils
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/src/UnfoldUtils-build
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/tmp
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/src/UnfoldUtils-stamp
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E make_directory /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/src
	/cvmfs/minerva.opensciencegrid.org/minerva/hep_hpc_products/cmake/v3_7_1/Linux64bit+3.10-2.17/bin/cmake -E touch /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-mkdir

UnfoldUtils-update: CMakeFiles/UnfoldUtils-update
UnfoldUtils-update: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-update
UnfoldUtils-update: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-download
UnfoldUtils-update: UnfoldUtils-prefix/src/UnfoldUtils-stamp/UnfoldUtils-mkdir
UnfoldUtils-update: CMakeFiles/UnfoldUtils-update.dir/build.make

.PHONY : UnfoldUtils-update

# Rule to build all files generated by this target.
CMakeFiles/UnfoldUtils-update.dir/build: UnfoldUtils-update

.PHONY : CMakeFiles/UnfoldUtils-update.dir/build

CMakeFiles/UnfoldUtils-update.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/UnfoldUtils-update.dir/cmake_clean.cmake
.PHONY : CMakeFiles/UnfoldUtils-update.dir/clean

CMakeFiles/UnfoldUtils-update.dir/depend:
	cd /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/MAT-MINERvA/bootstrap /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/MAT-MINERvA/bootstrap /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build /minerva/app/users/sultana/cmtuser/WorkingArea/LowRecoilPions/opt/build/CMakeFiles/UnfoldUtils-update.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/UnfoldUtils-update.dir/depend

