cmake_minimum_required(VERSION 3.14)
project(TicTacToeCourse VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# 1. Библиотека ядра (tttcore)
# ==========================================
file(GLOB_RECURSE TTT_CORE_SOURCES "src/core/*.cpp")
add_library(tttcore STATIC ${TTT_CORE_SOURCES})
target_include_directories(tttcore PUBLIC ${CMAKE_SOURCE_DIR}/src)

# ==========================================
# 2. Библиотека моего бота (tttplayer)
# ==========================================
# Добавляем my_bot.cpp в библиотеку
file(GLOB_RECURSE TTT_PLAYER_SOURCES "src/player/*.cpp" "src/my_bot.cpp")
add_library(tttplayer STATIC ${TTT_PLAYER_SOURCES})
target_include_directories(tttplayer PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tttplayer PUBLIC tttcore)

message(STATUS "CMake configured successfully!")