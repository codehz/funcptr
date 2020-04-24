cmake_minimum_required(VERSION 3.17)

project(funcptr)

add_library(funcptr INTERFACE)
target_include_directories(funcptr INTERFACE include)

add_executable(demo demo.cpp)
target_link_libraries(demo funcptr)
set_property(TARGET demo PROPERTY CXX_STANDARD 17)

add_executable(hackdemo hackdemo.cpp)
target_link_libraries(hackdemo funcptr)
set_property(TARGET hackdemo PROPERTY CXX_STANDARD 17)