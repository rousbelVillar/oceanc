cmake_minimum_required(VERSION 4.0)
project(oceanc)

set(CMAKE_CXX_STANDARD 20)

add_executable(oceanc main.cpp
        token_actions/token.h
        token_actions/token.cpp)
