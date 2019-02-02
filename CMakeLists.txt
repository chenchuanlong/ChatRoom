cmake_minimum_required(VERSION 3.13)
project(chat_room)

set(CMAKE_CXX_STANDARD 14)
add_executable(server ServerMain.cpp Server.h Server.cpp Common.h)
add_executable(client ClientMain.cpp Client.h Client.cpp Common.h)