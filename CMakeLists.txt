cmake_minimum_required(VERSION 3.10)

# Both Release and Debug will be built with -O3, but Release will set NDEBUG.
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.*;-checks=*")

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

add_library(llInstance src/ll/instance.cpp)
add_library(llPhysDev src/ll/phys_dev.cpp)
add_library(llQueue src/ll/queue.cpp)
add_library(llDevice src/ll/device.cpp)
add_library(llSwapchain src/ll/swapchain.cpp)
add_library(llImage src/ll/image.cpp)
add_library(llShader src/ll/shader.cpp)
add_library(llRpass src/ll/rpass.cpp)
add_library(llPipeline src/ll/pipeline.cpp)
add_library(llCbuf src/ll/cbuf.cpp)
add_library(llSync src/ll/sync.cpp)

add_library(GlfwWindow src/glfw_window.cpp)

# Add the executables
add_executable(Testing examples/testing.cpp)
add_executable(Triangle examples/triangle.cpp)

# Link
target_link_libraries(Testing Base)
target_link_libraries(Testing glfw)
target_link_libraries(Testing vulkan)
target_link_libraries(Testing llInstance)
target_link_libraries(Testing llPhysDev)
target_link_libraries(Testing llQueue)
target_link_libraries(Testing llDevice)
target_link_libraries(Testing llSwapchain)
target_link_libraries(Testing llImage)
target_link_libraries(Testing llShader)
target_link_libraries(Testing llRpass)
target_link_libraries(Testing llPipeline)
target_link_libraries(Testing llCbuf)
target_link_libraries(Testing llSync)
target_link_libraries(Testing GlfwWindow)

target_link_libraries(Triangle vulkan)
target_link_libraries(Triangle glfw)
target_link_libraries(Triangle Base)
target_link_libraries(Triangle llInstance)
target_link_libraries(Triangle llPhysDev)
target_link_libraries(Triangle llQueue)
target_link_libraries(Triangle llDevice)
target_link_libraries(Triangle GlfwWindow)
