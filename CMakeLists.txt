cmake_minimum_required(VERSION 4.2.3)
project(Todoer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(simdjson REQUIRED)
add_executable(task-cli
    src/main.cpp
    src/Storage.cpp
    src/Manager.cpp
)

target_link_libraries(task-cli PRIVATE simdjson::simdjson)
target_include_directories(task-cli PRIVATE include/)