cmake_minimum_required(VERSION 3.20.0)

add_executable(test_my_player test_my_player.cpp)
target_link_libraries(test_my_player tttplayer)
add_test(NAME smoke_test_player COMMAND ./test_my_player)

if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
add_executable(test_stats_baseline_easy test_stats_baseline_easy.cpp)
target_link_libraries(test_stats_baseline_easy tttplayer)
add_test(NAME test_player_stats_baseline_easy COMMAND ./test_stats_baseline_easy)
endif()

if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
add_executable(test_stats_baseline_hard test_stats_baseline_hard.cpp)
target_link_libraries(test_stats_baseline_hard tttplayer)
add_test(NAME test_player_stats_baseline_hard COMMAND ./test_stats_baseline_hard)
endif()

add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats tttplayer)
add_test(NAME test_player_stats COMMAND ./test_stats)

#if you are testing vs baseline add the cmake condition, just uncomment the lines
#if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
add_executable(test_my_player_vs_human test_my_player_vs_human.cpp)
target_link_libraries(test_my_player_vs_human tttplayer)
add_test(NAME test_my_player_vs_human COMMAND ./test_my_player_vs_human)
#endif()
if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
  add_executable(test_with_baseline test_baseline.cpp)
  target_link_libraries(test_with_baseline tttplayer)
  add_test(NAME test_againts_baseline COMMAND ./test_with_baseline)
endif()
