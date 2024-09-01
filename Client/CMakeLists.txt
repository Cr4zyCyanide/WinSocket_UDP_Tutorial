cmake_minimum_required(VERSION 3.26)
project(typing_battle_C C)

link_libraries(ws2_32)

set(CMAKE_C_STANDARD 11)

add_executable(Client main.c
        net.c
        net.h
        threads.c
        threads.h
        game.c
        game.h)
