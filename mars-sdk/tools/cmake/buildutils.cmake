cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

function(add_prev_build TGT DEP OUT)
    file(RELATIVE_PATH _dep_rel ${CMAKE_BINARY_DIR} ${DEP})
    file(RELATIVE_PATH _out_rel ${CMAKE_BINARY_DIR} ${OUT})
    file(RELATIVE_PATH _inc_rel ${CMAKE_BINARY_DIR} ${CMAKE_HOME_DIRECTORY}/include)
    file(TO_CMAKE_PATH "${_dep_rel}" _dep_rel)
    file(TO_CMAKE_PATH "${_out_rel}" _out_rel)
    file(TO_CMAKE_PATH "${_inc_rel}" _inc_rel)
    add_custom_target(
        ${TGT} ALL
        DEPENDS ${DEP}
        COMMENT "Pre-Building ..."
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_CPP_COMPILER} -E -P -undef ${_dep_rel} -I${_inc_rel} -o ${_out_rel}
    )
endfunction()

function(add_post_build TGT DEP OUT)
    file(RELATIVE_PATH _workdir_rel ${CMAKE_CURRENT_BINARY_DIR} ${EXECUTABLE_OUTPUT_PATH})
    file(RELATIVE_PATH _patch_rel ${EXECUTABLE_OUTPUT_PATH} ${CMAKE_HOME_DIRECTORY}/tools/binutils/bpatch.py)
    file(TO_CMAKE_PATH "${_workdir_rel}" _workdir_rel)
    file(TO_CMAKE_PATH "${_patch_rel}" _patch_rel)
    add_custom_target(
        ${TGT} ALL
        DEPENDS ${DEP}
        COMMENT "Post-Building ..."
        WORKING_DIRECTORY ${_workdir_rel}
        COMMAND ${PYTHON_CMD} ${_patch_rel} ${DEP}
        COMMAND ${CMAKE_UTILS_NM} -CSsnl -f sysv ${DEP} > ${OUT}.symb
        COMMAND ${CMAKE_UTILS_OBJCOPY} -Obinary ${DEP} ${OUT}.bin
        COMMAND ${CMAKE_UTILS_OBJCOPY} -Oihex ${DEP} ${OUT}.hex
        COMMAND ${CMAKE_UTILS_READELF} -a ${DEP} > ${OUT}.relf
        COMMAND ${CMAKE_UTILS_OBJDUMP} -d ${DEP} > ${OUT}.dasm
        COMMAND ${CMAKE_UTILS_SIZE} -B ${DEP}
    )
endfunction()

function(include_sub_header_dirs)
    # 使用 CMake 原生方式查找头文件目录（跨平台兼容）
    foreach(_DIR ${ARGV})
        file(GLOB_RECURSE _HEADERS "${_DIR}/*.h")
        foreach(_HDR ${_HEADERS})
            get_filename_component(_HDR_DIR ${_HDR} DIRECTORY)
            list(APPEND DIRS ${_HDR_DIR})
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES DIRS)
    include_directories(${DIRS})
endfunction()

function(find_and_add_subdirectory)
	file(GLOB _MKLISTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/CMakeLists.txt")
	foreach(_MKLST ${_MKLISTS})
		get_filename_component(_SUBDIR ${_MKLST} PATH)
		if(NOT "${_SUBDIR}" STREQUAL "")
			add_subdirectory(${_SUBDIR})
		endif()
	endforeach()
endfunction()
