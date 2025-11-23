cmake_minimum_required(VERSION 4.0)
project(mini_compilador)

set(CMAKE_CXX_STANDARD 20)

add_executable(mini_compilador main.cpp
        token_actions/token.h
        token_actions/token.cpp)
