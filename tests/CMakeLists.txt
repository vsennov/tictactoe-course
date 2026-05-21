cmake_minimum_required(VERSION 3.20.0)

# Always built targets (no dependency on BUILD_TTTCORE)
add_executable(test_my_player test_my_player.cpp)
target_link_libraries(test_my_player tttplayer)
add_test(NAME smoke_test_player COMMAND ./test_my_player)

add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats tttplayer)
add_test(NAME test_player_stats COMMAND ./test_stats)

# Targets that require full or prebuilt tttcore
if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
  # Baseline tests
  add_executable(test_with_baseline test_baseline.cpp)
  target_link_libraries(test_with_baseline tttplayer)
  add_test(NAME test_againts_baseline COMMAND ./test_with_baseline)

  # New stat test
  add_executable(test_stats_vs_baseline test_stats_vs_baseline.cpp)
  target_link_libraries(test_stats_vs_baseline tttplayer)
  add_test(NAME test_stats_vs_baseline COMMAND ./test_stats_vs_baseline)
  
  # Human player test
  add_executable(test_my_player_vs_human test_my_player_vs_human.cpp human_player.cpp)
  target_link_libraries(test_my_player_vs_human tttplayer)

  add_executable(test_stats_vs_harder test_stats_vs_harder.cpp)
  target_link_libraries(test_stats_vs_harder tttplayer)
  add_test(NAME test_stats_vs_harder COMMAND test_stats_vs_harder)

  add_executable(test_stats_vs_easy test_stats_vs_easy.cpp)
  target_link_libraries(test_stats_vs_easy tttplayer)
  add_test(NAME test_stats_vs_easy COMMAND test_stats_vs_easy)
endif()
