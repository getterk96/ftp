cmake_minimum_required(VERSION 3.7)
project(server)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES Server.c Server.h Commands.c IO.c Tools.c Tools.h)
add_executable(server ${SOURCE_FILES})

find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
