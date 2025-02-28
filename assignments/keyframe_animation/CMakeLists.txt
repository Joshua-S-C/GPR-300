file(
 GLOB_RECURSE ASSIGNMENT1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this keyframe_animation's asset folder to bin when it is built
add_custom_target(copyAssetsA3 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT1_INC} DESTINATION include/keyframe_animation)
add_executable(keyframe_animation ${ASSIGNMENT1_SRC} ${ASSIGNMENT1_INC})
target_link_libraries(keyframe_animation PUBLIC core IMGUI assimp)
target_include_directories(keyframe_animation PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when keyframe_animation is built
add_dependencies(keyframe_animation copyAssetsA3)