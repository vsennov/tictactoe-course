cmake_minimum_required(VERSION 3.20)

set(TTTPLAYER_SOURCES
tttplayer/my_player.cpp
tttplayer/line_scan.cpp
)

set(TTTPLAYER_HEADERS
tttplayer/my_player.hpp
tttplayer/line_scan.hpp
)

add_library(tttplayer ${TTTPLAYER_SOURCES} ${TTTPLAYER_HEADERS})

target_include_directories(tttplayer
PUBLIC
${CMAKE_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}
)

if(TARGET tttcore)
target_link_libraries(tttplayer PUBLIC tttcore)
else()
if(NOT TTTCORE_LIB)
message(FATAL_ERROR "TTTCORE_LIB is not set.")
endif()
target_link_libraries(tttplayer PUBLIC ${TTTCORE_LIB})
endif()