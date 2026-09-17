cmake_minimum_required(VERSION 3.16)
project(RenderComparison LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT RenderComparison)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../minifb" "${CMAKE_CURRENT_BINARY_DIR}/minifb")

add_library(RenderJob source/RenderJob.cpp source/RenderJob.h)
target_include_directories(RenderJob PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/source")

add_executable(RenderComparison source/main.cpp source/WindowLayout.cpp source/WindowLayout.h)
target_link_libraries(RenderComparison PRIVATE RenderJob minifb)
if (WIN32)
    target_link_libraries(RenderComparison PRIVATE opengl32 winmm)
endif()

enable_testing()
add_executable(RenderJobTests tests/RenderJobTests.cpp)
target_link_libraries(RenderJobTests PRIVATE RenderJob)
add_test(NAME RenderJobTests COMMAND RenderJobTests)
add_executable(WindowLayoutTests tests/WindowLayoutTests.cpp source/WindowLayout.cpp)
target_include_directories(WindowLayoutTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
add_test(NAME WindowLayoutTests COMMAND WindowLayoutTests)
