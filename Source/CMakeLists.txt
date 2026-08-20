cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Source)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/gom/gom_firmware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/app/router_application.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/scpi/router_scpi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/relay/relay_matrix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/board/board_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/board/board_shift_register.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/log/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/log/SEGGER_RTT.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/ieee488.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/minimal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/units.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/lexer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/src/expression.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/gom
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/app
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/scpi
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/relay
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/board
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/log
    ${CMAKE_CURRENT_SOURCE_DIR}/../App/thirdparty/scpi-parser/libscpi/inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LOG_ENABLE
    SCPI_USER_CONFIG
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
