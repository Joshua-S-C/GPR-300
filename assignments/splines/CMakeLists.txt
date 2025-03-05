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
#Copies this splines's asset folder to bin when it is built
add_custom_target(copyAssetsA4 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT1_INC} DESTINATION include/splines)
add_executable(splines ${ASSIGNMENT1_SRC} ${ASSIGNMENT1_INC})
target_link_libraries(splines PUBLIC core IMGUI assimp)
target_include_directories(splines PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when splines is built
add_dependencies(splines copyAssetsA4)