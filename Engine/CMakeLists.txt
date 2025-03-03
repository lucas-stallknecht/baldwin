cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(baldwin)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_DIR "${ROOT_DIR}/src")

add_library(${PROJECT_NAME} SHARED src/engine.cpp)

target_include_directories(${PROJECT_NAME} INTERFACE ${SOURCE_DIR})
