cmake_minimum_required (VERSION 2.8)
# 项目信息
project (JConfigParser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions("-Wall -g -O0")
include_directories(JConfigParser interface)

add_subdirectory(example)
add_subdirectory(ut)