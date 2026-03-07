cmake_minimum_required(VERSION 3.31)
project(chessAnalizer)

set(CMAKE_CXX_STANDARD 20)

add_executable(chessAnalizer main.cpp
        Board.cpp
        Solver.cpp
        pieces.cpp
        Game.cpp
        Board.h
)
