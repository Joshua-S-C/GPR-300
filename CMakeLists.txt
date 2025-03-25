cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# Include guards
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

project(JupiRender)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/libs)

include(external/cpm.cmake)

# add libraries
include(external/glfw.cmake)
include(external/imgui.cmake)
include(external/assimp.cmake)
include(external/glm.cmake)
include(external/imguizmo.cmake)

add_subdirectory(core)
add_subdirectory(assignments/assignment0)
add_subdirectory(assignments/assignment1_post_processing)
add_subdirectory(assignments/assignment2_shadow_map)
add_subdirectory(assignments/keyframe_animation)
add_subdirectory(assignments/splines)
add_subdirectory(assignments/fk)