cmake_minimum_required(VERSION 3.15)
project(FortuneRenderer LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# 项目本身的源文件：
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/*.h"
    "${CMAKE_SOURCE_DIR}/source/*.hpp"
)

set(SCENE_FILE "${CMAKE_SOURCE_DIR}/scene.xml")
set(CORNELL_SCENE_FILE "${CMAKE_SOURCE_DIR}/cornell_box.xml")
set(DUST2_SCENE_FILE "${CMAKE_SOURCE_DIR}/dust2.xml")
set(DUST2_MODEL_SCENE_FILE "${CMAKE_SOURCE_DIR}/dust2_model.xml")
configure_file(
    "${SCENE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/scene.xml"
    COPYONLY)
configure_file(
    "${CORNELL_SCENE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/cornell_box.xml"
    COPYONLY)
configure_file(
    "${DUST2_SCENE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/dust2.xml"
    COPYONLY)
configure_file(
    "${DUST2_MODEL_SCENE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/dust2_model.xml"
    COPYONLY)

add_executable(FortuneRenderer ${APP_SOURCES})
target_sources(FortuneRenderer PRIVATE
    "${SCENE_FILE}"
    "${CORNELL_SCENE_FILE}"
    "${DUST2_SCENE_FILE}"
    "${DUST2_MODEL_SCENE_FILE}")
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT FortuneRenderer)

# Keep symbols available when debugging the Release configuration in Visual Studio.
if (MSVC)
    target_compile_options(FortuneRenderer PRIVATE /utf-8)
    target_compile_options(FortuneRenderer PRIVATE $<$<CONFIG:Release>:/Zi>)
    target_link_options(FortuneRenderer PRIVATE $<$<CONFIG:Release>:/DEBUG:FULL>)
endif()
 
# 头文件附加目录：minifb/include 和 glm
target_include_directories(FortuneRenderer PRIVATE
    "${CMAKE_SOURCE_DIR}/minifb/include"
    "${CMAKE_SOURCE_DIR}/glm"
)
 
# 添加 minifb 到构建项目：
add_subdirectory(minifb)

target_link_libraries(FortuneRenderer PRIVATE minifb)

enable_testing()
add_executable(SphereTests tests/SphereTests.cpp source/Sphere.cpp)
target_include_directories(SphereTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
)
if (MSVC)
    target_compile_options(SphereTests PRIVATE /utf-8)
endif()
add_test(NAME SphereTests COMMAND SphereTests)

add_executable(DiskTests tests/DiskTests.cpp source/Disk.cpp)
target_include_directories(DiskTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
)
if (MSVC)
    target_compile_options(DiskTests PRIVATE /utf-8)
endif()
add_test(NAME DiskTests COMMAND DiskTests)

add_executable(TriangleTests tests/TriangleTests.cpp source/Triangle.cpp)
target_include_directories(TriangleTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
)
if (MSVC)
    target_compile_options(TriangleTests PRIVATE /utf-8)
endif()
add_test(NAME TriangleTests COMMAND TriangleTests)

add_executable(RenderTimingTests tests/RenderTimingTests.cpp source/RenderTiming.cpp)
target_include_directories(RenderTimingTests PRIVATE "${CMAKE_SOURCE_DIR}/source")
if (MSVC)
    target_compile_options(RenderTimingTests PRIVATE /utf-8)
endif()
add_test(NAME RenderTimingTests COMMAND RenderTimingTests)

add_executable(PrimitiveTests tests/PrimitiveTests.cpp)
target_include_directories(PrimitiveTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
)
if (MSVC)
    target_compile_options(PrimitiveTests PRIVATE /utf-8)
endif()
add_test(NAME PrimitiveTests COMMAND PrimitiveTests)

add_executable(SceneTests
    tests/SceneTests.cpp
    source/Disk.cpp
    source/Scene.cpp
    source/SceneObject.cpp
    source/Sphere.cpp
    source/Triangle.cpp
)
target_include_directories(SceneTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
)
if (MSVC)
    target_compile_options(SceneTests PRIVATE /utf-8)
endif()
add_test(NAME SceneTests COMMAND SceneTests)

add_executable(RendererTests
    tests/RendererTests.cpp
    source/Camera.cpp
    source/Disk.cpp
    source/RenderTiming.cpp
    source/Renderer.cpp
    source/Scene.cpp
    source/SceneObject.cpp
    source/Sphere.cpp
    source/Triangle.cpp
)
target_include_directories(RendererTests PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
    "${CMAKE_SOURCE_DIR}/glm"
    "${CMAKE_SOURCE_DIR}/minifb/include"
)
target_link_libraries(RendererTests PRIVATE minifb)
if (MSVC)
    target_compile_options(RendererTests PRIVATE /utf-8)
endif()
add_test(NAME RendererTests COMMAND RendererTests)

foreach(target FortuneRenderer SceneTests RendererTests)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/scene.xml"
            "$<TARGET_FILE_DIR:${target}>/scene.xml"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/cornell_box.xml"
            "$<TARGET_FILE_DIR:${target}>/cornell_box.xml"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/dust2.xml"
            "$<TARGET_FILE_DIR:${target}>/dust2.xml"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/dust2_model.xml"
            "$<TARGET_FILE_DIR:${target}>/dust2_model.xml")
endforeach()

if (WIN32)

# 链接库：
target_link_libraries(FortuneRenderer PRIVATE
    opengl32
    winmm
    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32)

endif()
