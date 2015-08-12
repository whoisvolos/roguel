cmake_minimum_required(VERSION 3.1)
project(playground)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp math.h maze.h maze.cpp graph.h)

find_package(Curses REQUIRED)

add_executable(playground ${SOURCE_FILES})

include_directories(playground ${CURSES_INCLUDE_DIR})

target_link_libraries(playground ${CURSES_LIBRARIES})