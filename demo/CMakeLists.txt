project (simple-svg-demo)
cmake_minimum_required(VERSION 3.5)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties (${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC simple-svg-writer)
