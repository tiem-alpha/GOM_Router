cmake_minimum_required(VERSION 3.20)
project(gom850_router C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_library(gom850_router
    App/relay/relay_matrix.c)
target_include_directories(gom850_router PUBLIC App/core App/gom App/app App/middle/scpi App/relay App/board Source/thirdparty/scpi-parser/libscpi/inc)
target_sources(gom850_router PRIVATE
    Source/thirdparty/scpi-parser/libscpi/src/error.c
    Source/thirdparty/scpi-parser/libscpi/src/fifo.c
    Source/thirdparty/scpi-parser/libscpi/src/ieee488.c
    Source/thirdparty/scpi-parser/libscpi/src/minimal.c
    Source/thirdparty/scpi-parser/libscpi/src/parser.c
    Source/thirdparty/scpi-parser/libscpi/src/units.c
    Source/thirdparty/scpi-parser/libscpi/src/utils.c
    Source/thirdparty/scpi-parser/libscpi/src/lexer.c
    Source/thirdparty/scpi-parser/libscpi/src/expression.c)
if (MSVC)
  target_compile_options(gom850_router PRIVATE /W4 /WX)
else()
  target_compile_options(gom850_router PRIVATE -Wall -Wextra -Werror -Wpedantic)
endif()

add_executable(gom850_host tests/host/test_router.c)
target_link_libraries(gom850_host PRIVATE gom850_router)
enable_testing()
add_test(NAME gom850_host COMMAND gom850_host)
