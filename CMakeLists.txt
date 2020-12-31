cmake_minimum_required(VERSION 3.10)

# Both Release and Debug will be built with -O3, but Release will set NDEBUG.
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the project name
project(RenderCpp)

# Compile options
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # lots of warnings
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Add libraries
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
add_library(VkInstance vk_instance.cpp)
add_library(VkPhysDev vk_phys_dev.cpp)
add_library(VkQueue vk_queue.cpp)
add_library(VkDevice vk_device.cpp)

# Add the executable
add_executable(Testing testing.cpp)

# Link
target_link_libraries(Testing glfw)
target_link_libraries(Testing vulkan)
target_link_libraries(Testing VkInstance)
target_link_libraries(Testing VkPhysDev)
target_link_libraries(Testing VkQueue)
target_link_libraries(Testing VkDevice)

