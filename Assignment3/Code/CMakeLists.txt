cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies")

include_directories(BEFORE "${DEPS_DIR}/mingw_thread_compat")

set(EIGEN3_INCLUDE_DIR "${DEPS_DIR}/eigen-3.4.0")
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories("${DEPS_DIR}")

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp Texture.hpp Texture.cpp Shader.hpp OBJ_Loader.h)
target_link_libraries(Rasterizer ${OpenCV_LIBS})
