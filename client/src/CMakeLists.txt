cmake_minimum_required(VERSION 3.7)
project(client)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES Client.c Client.h IO.c Tools.c Commands.c)
add_executable(client ${SOURCE_FILES})

find_package(Threads REQUIRED)
target_link_libraries(client Threads::Threads)
