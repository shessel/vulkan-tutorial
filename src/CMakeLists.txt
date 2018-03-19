cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)
cmake_policy(VERSION 3.7.0)

project(vulkan-tutorial)

# Use FindVulkan module added with CMAKE 3.7
message(STATUS "Using module to find Vulkan")
find_package(Vulkan)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Werror")
add_definitions(-std=c++17)

include_directories(${Vulkan_INCLUDE_DIRS}
                    ${GLFW_SOURCE_DIR}/include
                    ${GLM_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SOURCES
    hello-triangle.cpp
    render/vulkan/Device.cpp
)

add_executable(hello-triangle ${SOURCES})
target_link_libraries(hello-triangle ${Vulkan_LIBRARY} glfw ${GLFW_LIBRARIES})
