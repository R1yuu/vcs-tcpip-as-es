cmake_minimum_required(VERSION 3.16)
project(vcs_tcpip_as_es C)

set(CMAKE_C_STANDARD 11)
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11")  
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(vcs_tcpip_as_es
        simple_message_server.c
)

add_custom_command(TARGET vcs_tcpip_as_es 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ../required_binaries/* .)