cmake_minimum_required(VERSION 3.16)
project(vcs_tcpip_as_es C)

set(CMAKE_C_STANDARD 11)

add_executable(vcs_tcpip_as_es
        simple_message_server.c
)

add_custom_command(TARGET vcs_tcpip_as_es 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ../required_binaries/* .)