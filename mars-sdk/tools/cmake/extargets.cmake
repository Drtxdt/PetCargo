cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

add_custom_target(
    menuconfig
    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    COMMENT "MenuConfig"
    COMMAND ${KCONF_CMD} -K${KCONF_SRC} -H${KCONF_HDR} -C${KCONF_CFG} -M
    USES_TERMINAL
)

add_custom_target(
    defconfig
    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    COMMENT "DefConfig"
    COMMAND ${KCONF_CMD} -K${KCONF_SRC} -H${KCONF_HDR} -C${KCONF_CFG} -D${KCONF_DEF}
    USES_TERMINAL
)

add_custom_target(
    respack
    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    COMMENT "ResourcePack"
    COMMAND ${GEOTP_CMD} ../respack/otptab.bin
    COMMAND ${RPACK_CMD} -p config.ini
    # COMMAND ${RPACK_CMD} -u respak.bin outs
    USES_TERMINAL
)

add_custom_target(
    burn
    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    COMMENT "Program ..."
    COMMAND JFlash.exe -openprjtools/jflash/mars.jflash -openbuild/bin/app.bin,0x30000000 -auto -exit -hide
    # COMMAND JFlash.exe -openprjtools/jflash/mars.jflash -opentools/respack/respak.bin,0x30080000 -auto -exit 
)
