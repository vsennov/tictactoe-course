cmake_minimum_required(VERSION 3.20.0)

project(tictactoe-lab LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_EXPORT_COMPILE_COMMANDS)
  set (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# NOTE: set this variable to "PREBUILT" when you downloaded prebuilt tttcore
# library with baseline player.
#set(BUILD_TTTCORE "PREBUILT")

if (BUILD_TTTCORE STREQUAL "PREBUILT")
  find_library(TTTCORE_LIB libtttcore.a PATHS libs)

  if (NOT TTTCORE_LIB)
    message (FATAL_ERROR 
        "Cannot find prebuilt tttcore library. Please, make sure that you have "
        "placed it in `libs/` directory. If you have no prebuilt library, build "
        "your code without baseline from available sources (without setting "
        "BUILD_TTTCORE varaible)."
      )
  endif()
else()
  set(core_src src/core/event.cpp src/core/game.cpp src/core/state.cpp)
  if (BUILD_TTTCORE STREQUAL "FULL")
    set(core_src ${core_src} ../baseline.cpp)
  endif()
  add_library(tttcore STATIC ${core_src})
  set(TTTCORE_LIB tttcore)
endif()

include_directories(src)

# NOTE: add source files for your players here
set(player_src src/player/my_player.cpp src/player/my_observer.cpp)
add_library(tttplayer STATIC ${player_src})
target_link_libraries(tttplayer ${TTTCORE_LIB})

# NOTE: enable or disable ctest
enable_testing()
add_subdirectory("tests")

# NOTE: this submodule builds sources for remote game
if (BUILD_TTTREMOTE)
  add_subdirectory("src/remote")
endif()
