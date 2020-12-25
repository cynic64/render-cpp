cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Release)

# Set the project name
project(RenderCpp)

# Compile options
if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Add libraries
find_package(glfw3 REQUIRED)

# Add the executable
add_executable(Testing testing.cpp)

# Link
target_link_libraries(Testing glfw)

