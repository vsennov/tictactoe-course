#pragma once

#include "core/game.hpp"
#include <iostream>
#include <cassert>
#include <ctime>
#include <string>

namespace ttt::test {

// Average counter utility
class AverageCounter {
public:
  void set(double val) {
    total_sum += val;
    ++n_measures;
  }
  
  double get() const { 
    return n_measures > 0 ? total_sum / n_measures : 0; 
  }

private:
  int n_measures = 0;
  double total_sum = 0;
};

// Time measurement helper
class BlockMeasurer {
public:
  BlockMeasurer(AverageCounter &counter) : counter(counter), start(std::clock()) {}
  
  ~BlockMeasurer() {
    auto end = std::clock();
    counter.set(double(end - start) / CLOCKS_PER_SEC * 1000);
  }

private:
  AverageCounter &counter;
  clock_t start;
};

// Player wrapper that measures execution time
class TimeMeasuringPlayer : public game::IPlayer {
public:
  TimeMeasuringPlayer(game::IPlayer &p) : m_base(p) {}

  void set_sign(game::Sign sign) override { 
    m_base.set_sign(sign); 
  }

  game::Point make_move(const game::State &state) override {
    BlockMeasurer ms{m_move_time};
    return m_base.make_move(state);
  }

  void handle_event(const game::State &state, const game::Event &event) override {
    BlockMeasurer ms{m_event_time};
    m_base.handle_event(state, event);
  }

  const char *get_name() const override { 
    return m_base.get_name(); 
  }

  double get_average_move_time() const { 
    return m_move_time.get(); 
  }
  
  double get_average_event_time() const { 
    return m_event_time.get(); 
  }

private:
  game::IPlayer &m_base;
  AverageCounter m_move_time;
  AverageCounter m_event_time;
};

// Main test function that runs games and collects statistics
struct TestResult {
    int x_wins = 0;
    int o_wins = 0;
    int draws = 0;
    int errors = 0;
    double x_move_time = 0;
    double x_event_time = 0;
    double o_move_time = 0;
    double o_event_time = 0;
    double game_time = 0;
};

static TestResult run_game_tests(
    game::IPlayer& p1, 
    game::IPlayer& p2, 
    int num_iterations = 100,
    int board_size = 15,
    int win_length = 5) {
    
    //game options
    game::State::Opts opts;
    opts.rows = opts.cols = board_size;
    opts.win_len = win_length;
    opts.max_moves = 0;
    
    // Wrap players with time measurement
    TimeMeasuringPlayer tm_p1(p1), tm_p2(p2);
    
    
    game::Game game(opts);
    game.add_player(game::Sign::X, &tm_p1);
    game.add_player(game::Sign::O, &tm_p2);
    
    //prepare result counters
    TestResult result;
    AverageCounter game_time_counter;
    
    //run
    for (int i = 0; i < num_iterations; ++i) {
        game::MoveResult res;
        do {
            BlockMeasurer ms{game_time_counter};
            res = game.process();
        } while (res == game::MoveResult::OK);
        
        assert(!game::is_dq(res));
        
        switch (res) {
        case game::MoveResult::DRAW:
            ++result.draws;
            break;
        case game::MoveResult::ERROR:
            ++result.errors;
            break;
        default:
            if (game.get_state().get_winner() == game::Sign::X)
                ++result.x_wins;
            else if (game.get_state().get_winner() == game::Sign::O)
                ++result.o_wins;
            else
                ++result.errors;
        }
        game.reset();
    }
    
    //timing results
    result.x_move_time = tm_p1.get_average_move_time();
    result.x_event_time = tm_p1.get_average_event_time();
    result.o_move_time = tm_p2.get_average_move_time();
    result.o_event_time = tm_p2.get_average_event_time();
    result.game_time = game_time_counter.get();
    
    return result;
}

//helper to print test results
static void print_test_results(const TestResult& result, 
                              const std::string& player_x_name = "X",
                              const std::string& player_o_name = "O") {
    std::cout << player_x_name << " wins: " << result.x_wins << "\n"
              << player_o_name << " wins: " << result.o_wins << "\n"
              << "draws:  " << result.draws << "\n"
              << "errors: " << result.errors << "\n"
              << std::endl;

    std::cout << player_x_name << " play time:\n - move time (ms): " 
              << result.x_move_time
              << "\n - event time (ms): " << result.x_event_time
              << "\n\n";

    std::cout << player_o_name << " play time:\n - move time (ms): "
              << result.o_move_time
              << "\n - event time (ms): " << result.o_event_time
              << "\n\n";

    std::cout << "game process average time: " << result.game_time
              << " (ms)\n";
              
    assert(result.x_event_time < 100);
    assert(result.x_move_time < 100);
}

} // namespace ttt::test