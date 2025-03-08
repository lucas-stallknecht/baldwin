cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(baldwin)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_DIR ${ROOT_DIR}/src)
set(THIRD_PARTY_DIR ${ROOT_DIR}/third_party)

file(GLOB_RECURSE SOURCES
        "${SOURCE_DIR}/*.cpp"
        "${SOURCE_DIR}/*.c"
)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

find_package(Vulkan REQUIRED)

add_subdirectory(${THIRD_PARTY_DIR}/glfw-3.4)
add_subdirectory(${THIRD_PARTY_DIR}/vk-bootstrap)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${ROOT_DIR}/include
    ${SOURCE_DIR} 
    ${THIRD_PARTY_DIR}/glfw-3.4/include
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARIES} 
        vk-bootstrap::vk-bootstrap)
