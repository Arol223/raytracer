cmake_minimum_required(VERSION 3.25)
project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

add_executable(raytracer src/main.cpp)
target_compile_options(raytracer PRIVATE -Wall -Wextra -Wpedantic)
add_library(rt INTERFACE)
find_package(Threads REQUIRED)
target_include_directories(rt INTERFACE include)
target_link_libraries(raytracer PRIVATE rt)
target_link_libraries(rt INTERFACE Threads::Threads)
