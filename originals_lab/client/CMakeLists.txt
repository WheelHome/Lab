cmake_minimum_required(VERSION 2.8)
project(ccLogger)

# Flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(./)
aux_source_directory(./ccsocket ccSocket)
add_executable(Client ./main.cpp ${ccSocket})
