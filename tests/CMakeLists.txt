cmake_minimum_required(VERSION 3.20.0)

add_executable(test_my_player test_my_player.cpp)
target_link_libraries(test_my_player tttplayer)
add_test(NAME smoke_test_player COMMAND ./test_my_player)

add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats tttplayer)
add_test(NAME test_player_stats COMMAND ./test_stats)

if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
  add_executable(test_with_baseline test_baseline.cpp)
  target_link_libraries(test_with_baseline tttplayer)
  add_test(NAME test_againts_baseline COMMAND ./test_with_baseline)
endif()
