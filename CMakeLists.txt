cmake_minimum_required(VERSION 3.20)
project(gom850_router C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_library(gom850_router
    App/gom/gom_router.c
    App/gom/gom_command_encoder.c
    App/gom/gom_response.c
    App/relay/relay_matrix.c)
target_include_directories(gom850_router PUBLIC App/gom App/relay App/board)
if (MSVC)
  target_compile_options(gom850_router PRIVATE /W4 /WX)
else()
  target_compile_options(gom850_router PRIVATE -Wall -Wextra -Werror -Wpedantic)
endif()

add_executable(gom850_host tests/host/test_router.c)
target_link_libraries(gom850_host PRIVATE gom850_router)
enable_testing()
add_test(NAME gom850_host COMMAND gom850_host)
