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

add_library(Base src/base.cpp)

add_library(VkInstance src/ll/instance.cpp)
add_library(VkPhysDev src/ll/phys_dev.cpp)
add_library(VkQueue src/ll/queue.cpp)
add_library(VkDevice src/ll/device.cpp)
add_library(VkSwapchain src/ll/swapchain.cpp)
add_library(VkImage src/ll/image.cpp)
add_library(VkShader src/ll/shader.cpp)
add_library(VkRpass src/ll/rpass.cpp)
add_library(VkPipeline src/ll/pipeline.cpp)
add_library(VkCbuf src/ll/cbuf.cpp)

add_library(GlfwWindow src/glfw_window.cpp)

# Add the executables
add_executable(Testing examples/testing.cpp)
add_executable(Triangle examples/triangle.cpp)

# Link
target_link_libraries(Testing Base)
target_link_libraries(Testing glfw)
target_link_libraries(Testing vulkan)
target_link_libraries(Testing VkInstance)
target_link_libraries(Testing VkPhysDev)
target_link_libraries(Testing VkQueue)
target_link_libraries(Testing VkDevice)
target_link_libraries(Testing VkSwapchain)
target_link_libraries(Testing VkImage)
target_link_libraries(Testing VkShader)
target_link_libraries(Testing VkRpass)
target_link_libraries(Testing VkPipeline)
target_link_libraries(Testing VkCbuf)
target_link_libraries(Testing GlfwWindow)

target_link_libraries(Triangle vulkan)
target_link_libraries(Triangle glfw)
target_link_libraries(Triangle Base)
target_link_libraries(Triangle VkInstance)
target_link_libraries(Triangle VkPhysDev)
target_link_libraries(Triangle VkQueue)
target_link_libraries(Triangle VkDevice)
target_link_libraries(Triangle GlfwWindow)
