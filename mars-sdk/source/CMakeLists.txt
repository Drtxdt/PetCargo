cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

get_filename_component(TARGET ${PROJECT_NAME} NAME_WE)
set(LDSPATH ${EXECUTABLE_OUTPUT_PATH}/${TARGET}.lds)
file(RELATIVE_PATH LDSPATH_REL ${CMAKE_BINARY_DIR} ${LDSPATH})
file(TO_CMAKE_PATH "${LDSPATH_REL}" LDSPATH_REL)
add_prev_build(lds ${CMAKE_HOME_DIRECTORY}/source/startup/flash.ld ${LDSPATH})

file(GLOB_RECURSE _SSRCS "*.c" "*.S")
file(RELATIVE_PATH LIBDIR_REL ${CMAKE_BINARY_DIR} ${CMAKE_HOME_DIRECTORY}/libraries)
file(TO_CMAKE_PATH "${LIBDIR_REL}" LIBDIR_REL)
add_executable(${PROJECT_NAME} ${_CSRCS} ${_SSRCS})
add_dependencies(${PROJECT_NAME} lds)
target_link_options(${PROJECT_NAME} PRIVATE -T${LDSPATH_REL} -L${LIBDIR_REL})

list(APPEND _MODS bsp)
list(APPEND _MODS drv)
list(APPEND _MODS rtos)
list(APPEND _MODS crc32)
list(APPEND _MODS shell)
list(APPEND _MODS tnvs)
list(APPEND _MODS esr)
list(APPEND _MODS luna copy)
list(APPEND _MODS mp3)
message("-- Selected: ${_MODS}")

add_post_build(post ${PROJECT_NAME} ${TARGET})
target_link_libraries(${PROJECT_NAME} PRIVATE ${_MODS})
target_link_libraries(${PROJECT_NAME} PRIVATE m)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_sub_header_dirs(
	${CMAKE_CURRENT_SOURCE_DIR}/mods
	${CMAKE_CURRENT_SOURCE_DIR}/resmgr
	${CMAKE_CURRENT_SOURCE_DIR}/tasks
	${CMAKE_CURRENT_SOURCE_DIR}/startup
	${CMAKE_CURRENT_SOURCE_DIR}/listen
	${CMAKE_CURRENT_SOURCE_DIR}/listen/app
	${CMAKE_CURRENT_SOURCE_DIR}/listen/player
	${CMAKE_CURRENT_SOURCE_DIR}/listen/porting
	${CMAKE_CURRENT_SOURCE_DIR}/listen/model
	${CMAKE_CURRENT_SOURCE_DIR}/listen/utils
	${CMAKE_CURRENT_SOURCE_DIR}/listen/pa
	${CMAKE_HOME_DIRECTORY}/include
	${CMAKE_HOME_DIRECTORY}/modules
	${CMAKE_HOME_DIRECTORY}/modules/mp3
	${CMAKE_HOME_DIRECTORY}/chip
	${CMAKE_HOME_DIRECTORY}/libraries/include
)
