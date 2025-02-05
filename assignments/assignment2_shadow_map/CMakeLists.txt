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
#Copies this assignment2_shadow_map's asset folder to bin when it is built
add_custom_target(copyAssetsA2 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT1_INC} DESTINATION include/assignment2_shadow_map)
add_executable(assignment2_shadow_map ${ASSIGNMENT1_SRC} ${ASSIGNMENT1_INC})
target_link_libraries(assignment2_shadow_map PUBLIC core IMGUI assimp)
target_include_directories(assignment2_shadow_map PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment2_shadow_map is built
add_dependencies(assignment2_shadow_map copyAssetsA2)