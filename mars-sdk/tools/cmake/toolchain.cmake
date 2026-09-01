cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# 设置交叉编译目标系统为通用嵌入式（非Windows/Linux）
# 这会阻止CMake自动添加Windows特有的链接选项
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)

set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_C_COMPILER_ID_RUN BOOL FORCE)
set(CMAKE_ASM_COMPILER_ID_RUN BOOL FORCE)
set(CMAKE_CXX_COMPILER_ID_RUN BOOL FORCE)

set(CMAKE_C_COMPILER_ABI BOOL FORCE)
set(CMAKE_ASM_COMPILER_ABI BOOL FORCE)
set(CMAKE_CXX_COMPILER_ABI BOOL FORCE)

set(CMAKE_C_COMPILE_FEATURES BOOL FORCE)
set(CMAKE_ASM_COMPILE_FEATURES BOOL FORCE)
set(CMAKE_CXX_COMPILE_FEATURES BOOL FORCE)

set(CMAKE_C_COMPILER_WORKS TRUE CACHE BOOL "Set to TRUE if the C Compiler is Working" FORCE)
set(CMAKE_ASM_COMPILER_WORKS TRUE CACHE BOOL "Set to TRUE if the ASM Compiler is Working" FORCE)
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE BOOL "Set to TRUE if the CXX Compiler is Working" FORCE)

set(CROSSTOOL ${CONFIG_TOOLCHAIN_PREFIX})
string(CONCAT CMAKE_C_COMPILER      ${CROSSTOOL} gcc)
string(CONCAT CMAKE_ASM_COMPILER    ${CROSSTOOL} gcc)
string(CONCAT CMAKE_CXX_COMPILER    ${CROSSTOOL} g++)
string(CONCAT CMAKE_CPP_COMPILER    ${CROSSTOOL} cpp)
string(CONCAT CMAKE_UTILS_LINKER    ${CROSSTOOL} ld)
string(CONCAT CMAKE_UTILS_NM        ${CROSSTOOL} nm)
string(CONCAT CMAKE_UTILS_AR        ${CROSSTOOL} ar)
string(CONCAT CMAKE_UTILS_AS        ${CROSSTOOL} as)
string(CONCAT CMAKE_UTILS_SIZE      ${CROSSTOOL} size)
string(CONCAT CMAKE_UTILS_OBJCOPY   ${CROSSTOOL} objcopy)
string(CONCAT CMAKE_UTILS_OBJDUMP   ${CROSSTOOL} objdump)
string(CONCAT CMAKE_UTILS_READELF   ${CROSSTOOL} readelf)
string(CONCAT CMAKE_UTILS_RANLIB    ${CROSSTOOL} ranlib)
string(CONCAT CMAKE_UTILS_STRIP     ${CROSSTOOL} strip)
string(CONCAT CMAKE_UTILS_ADDR2LINE ${CROSSTOOL} addr2line)
string(CONCAT CMAKE_UTILS_STRINGS   ${CROSSTOOL} strings)
string(CONCAT CMAKE_UTILS_GDB       ${CROSSTOOL} gdb)
string(CONCAT CMAKE_UTILS_GCOV      ${CROSSTOOL} gcov)

set(CMAKE_C_FLAGS -std=gnu11)
set(CMAKE_ASM_FLAGS -xassembler-with-cpp)
set(CMAKE_CXX_FLAGS -std=gnu++17)
set(CMAKE_CPP_FLAGS "-E -P -undef")
set(PLATFORM_FLAGS -march=rv32imafc -mabi=ilp32f -mcmodel=medlow -mtune=nuclei-300-series)

add_link_options(${PLATFORM_FLAGS})
add_link_options(-static -nostartfiles --specs=nosys.specs -lnosys -fno-builtin-printf -Wl,--gc-sections)
string(FIND ${CROSSTOOL} riscv64-unknown-elf- _INDEX)
if(${_INDEX} GREATER -1)
    add_link_options(-Wl,--no-warn-rwx-segments)
endif()

add_compile_options(${PLATFORM_FLAGS})
add_compile_options(${CONFIG_TOOLCHAIN_OPTIMIZE})
add_compile_options(-Werror -Wall -Wno-comment -MMD -ffunction-sections -fdata-sections -ffast-math 
    -fno-common -fno-builtin-printf -fmessage-length=0 -fdiagnostics-show-option)
if(CONFIG_BTRACE_METHOD_OMIT_FP)
    add_compile_options(-fomit-frame-pointer)
elseif(CONFIG_BTRACE_METHOD_WITH_FP)
    add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls)
endif()

include_directories(${CMAKE_BINARY_DIR})
add_compile_definitions(IC_BOARD=1 CFG_RTOS=1)

# file(GLOB_RECURSE _RESF respak.bin PATH)
file(GLOB_RECURSE _RESF RELATIVE ${EXECUTABLE_OUTPUT_PATH} respak.bin)
add_compile_definitions(__RESFILE__="${_RESF}")
