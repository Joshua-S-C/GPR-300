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
#Copies this assignment1_post_processing's asset folder to bin when it is built
add_custom_target(copyAssetsA1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT1_INC} DESTINATION include/assignment1_post_processing)
add_executable(assignment1_post_processing ${ASSIGNMENT1_SRC} ${ASSIGNMENT1_INC})
target_link_libraries(assignment1_post_processing PUBLIC core IMGUI assimp)
target_include_directories(assignment1_post_processing PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment1_post_processing is built
add_dependencies(assignment1_post_processing copyAssetsA1)