# CMakeList.txt : CMake project for Sudoku, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_BUILD_TYPE Release)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

# Add source to this project's executable.
add_executable (Sudoku "Sudoku.cpp" "Sudoku.h")

# TODO: Add tests and install targets if needed.
