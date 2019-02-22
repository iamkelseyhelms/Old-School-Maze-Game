cmake_minimum_required(VERSION 3.6)
project(CorrectAdventure)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES helmsk.adventure.c)
add_executable(CorrectAdventure ${SOURCE_FILES})