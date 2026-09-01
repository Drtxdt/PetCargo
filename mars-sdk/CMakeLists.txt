cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools/cmake)

foreach(_CMKMOD envsetup toolchain buildutils extargets)
    include(${_CMKMOD})
endforeach()

project(app.elf VERSION 1.0.0 LANGUAGES C ASM)

find_and_add_subdirectory()
