cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# 自动检测 Python3 可执行文件路径（跨平台兼容）
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    set(PYTHON_CMD ${Python3_EXECUTABLE})
else()
    # 回退：尝试使用 python 或 python3
    find_program(PYTHON_CMD NAMES python3 python)
endif()

set(KCONF_CMD ${PYTHON_CMD} ${CMAKE_HOME_DIRECTORY}/tools/kconfpy/kconfig.py CACHE INTERNAL "" FORCE)
set(RPACK_CMD ${PYTHON_CMD} ${CMAKE_HOME_DIRECTORY}/tools/respack/respack.py CACHE INTERNAL "" FORCE)
set(PATCH_CMD ${PYTHON_CMD} ${CMAKE_HOME_DIRECTORY}/tools/binutils/bpatch.py CACHE INTERNAL "" FORCE)
set(GEOTP_CMD ${PYTHON_CMD} ${CMAKE_HOME_DIRECTORY}/tools/binutils/genotp.py CACHE INTERNAL "" FORCE)
set(KCONF_SRC Kconfig CACHE INTERNAL "" FORCE)
set(KCONF_CFG .config CACHE INTERNAL "" FORCE)
set(KCONF_HDR include/autoconf.h CACHE INTERNAL "" FORCE)
set(KCONF_DEF boards/defconfig CACHE INTERNAL "" FORCE)

function(kconfig_boards_enum)
    file(GLOB _CFGS "boards/*-defconfig")
    foreach(_PATH ${_CFGS})
        get_filename_component(_BCFG ${_PATH} NAME)
        file(RELATIVE_PATH _RPATH ${CMAKE_HOME_DIRECTORY} ${_PATH})
        add_custom_target(
            ${_BCFG}
            WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
            COMMENT "${_BCFG}"
            COMMAND ${KCONF_CMD} -K${KCONF_SRC} -H${KCONF_HDR} -C${KCONF_DEF} -D${_RPATH}
            USES_TERMINAL
        )
    endforeach()
endfunction()

function(kconfig_env_parse)
    if(EXISTS ${KCONF_CFG})
        file(STRINGS ${KCONF_CFG} _KCONF)
    else()
        if(NOT EXISTS ${KCONF_DEF})
            execute_process(COMMAND ${KCONF_CMD} -K${KCONF_SRC} -D${KCONF_DEF} -H${KCONF_HDR})
        endif()
        file(STRINGS ${KCONF_DEF} _KCONF)
    endif()

    foreach(_LINE in lists ${_KCONF})
        string(REGEX MATCH "^([^=]+)=(.*)$" _MATCH ${_LINE})
        if(_MATCH)
            set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2} CACHE INTERNAL "from kconfig")
        endif()
    endforeach()
endfunction()

kconfig_boards_enum()
kconfig_env_parse()

if(NOT ${CONFIG_TOOLCHAIN_PREFIX} STREQUAL "")
    string(REPLACE "\"" "" CONFIG_TOOLCHAIN_PREFIX ${CONFIG_TOOLCHAIN_PREFIX})
endif()
if(NOT ${CONFIG_TOOLCHAIN_OPTIMIZE} STREQUAL "")
    string(REPLACE "\"" "" CONFIG_TOOLCHAIN_OPTIMIZE ${CONFIG_TOOLCHAIN_OPTIMIZE})
endif()
