cmake_minimum_required(VERSION 3.9)

project(uka)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(uka-vulkan)
add_subdirectory(3rdparty)
add_executable(uka src/main.cpp)
target_link_libraries(uka uka_vulkan)
