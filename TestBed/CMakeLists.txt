cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(testbed)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
cmake_path(GET ROOT_DIR PARENT_PATH PARENT_DIR)
set(ENGINE_DIR ${PARENT_DIR}/Engine/) # Update this path if necessary

add_subdirectory(${ENGINE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/engine)
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} baldwin)
