cmake_minimum_required(VERSION 3.26)
project(typing_battle_S C)

link_libraries(ws2_32)

set(CMAKE_C_STANDARD 11)

add_executable(Server main.c
        files.h
        files.c
        net.h
        net.c
        game.h
        game.c
        threads.c
        threads.h)
