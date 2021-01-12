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

add_library(VkBase vk_base.cpp)

add_library(VkInstance vk_instance.cpp)
add_library(VkPhysDev vk_phys_dev.cpp)
add_library(VkQueue vk_queue.cpp)
add_library(VkDevice vk_device.cpp)
add_library(VkSwapchain vk_swapchain.cpp)
add_library(VkImage vk_image.cpp)
add_library(VkShader vk_shader.cpp)
add_library(VkRpass vk_rpass.cpp)
add_library(VkPipeline vk_pipeline.cpp)
add_library(VkCbuf vk_cbuf.cpp)

add_library(GlfwWindow glfw_window.cpp)

# Add the executables
add_executable(Testing testing.cpp)
add_executable(Triangle triangle.cpp)

# Link
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

target_link_libraries(Triangle VkBase)
target_link_libraries(Triangle VkInstance)
target_link_libraries(Triangle vulkan)

