#include "player/my_player.hpp"
#include "core/baseline.hpp"


#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

struct AverageCounter {
  int n_measures = 0;
  double total_sum = 0;

  void set(double val) {
    total_sum += val;
    ++n_measures;
  }
  double get() const { return n_measures > 0 ? total_sum / n_measures : 0; }
};

struct BlockMeasurer {
  AverageCounter &counter;
  clock_t start = std::clock();

  ~BlockMeasurer() {
    auto end = std::clock();
    counter.set(double(end - start) / CLOCKS_PER_SEC * 1000);
  }
};

class TimeMeasuringPlayer : public ttt::game::IPlayer {
  ttt::game::IPlayer &m_base;
  AverageCounter m_move_time;
  AverageCounter m_event_time;

public:
  TimeMeasuringPlayer(ttt::game::IPlayer &p) : m_base(p) {}

  void set_sign(ttt::game::Sign sign) override { m_base.set_sign(sign); }

  ttt::game::Point make_move(const ttt::game::State &state) override {
    BlockMeasurer ms{m_move_time};
    return m_base.make_move(state);
  }

  void handle_event(const ttt::game::State &state,
                    const ttt::game::Event &event) override {
    BlockMeasurer ms{m_event_time};
    m_base.handle_event(state, event);
  }

  const char *get_name() const override { return m_base.get_name(); }

  double get_average_move_time() const { return m_move_time.get(); }
  double get_average_event_time() const { return m_event_time.get(); }
};

int main(int argc, char *argv[]) {
  std::cout << "Testing Myplayer vs Baseline hard!\n";
  if (argc >= 2) {
    std::srand(atoi(argv[1]));
  }

  ttt::game::State::Opts opts;
  opts.rows = opts.cols = 15;
  opts.win_len = 5;
  opts.max_moves = 0;

  ttt::my_player::MyPlayer p1("p1"); // add your player here <~~~~~
  ttt::game::IPlayer *p2 = ttt::baseline::get_harder_player("p2");
  TimeMeasuringPlayer tm_p1(p1), tm_p2(*p2);

  ttt::game::Game game(opts);
  game.add_player(ttt::game::Sign::X, &tm_p1);
  game.add_player(ttt::game::Sign::O, &tm_p2);

  int n_x_wins = 0, n_o_wins = 0, n_draws = 0, n_errors = 0;

  AverageCounter game_time_counter;

  for (int i = 0; i < 100 /* <~~~ adjust number of sample tests here */; ++i) { 
    ttt::game::MoveResult res;
    do {
      BlockMeasurer ms{game_time_counter};
      res = game.process();
    } while (res == ttt::game::MoveResult::OK);
    assert(!ttt::game::is_dq(res));
    switch (res) {
    case ttt::game::MoveResult::DRAW:
      ++n_draws;
      break;
    case ttt::game::MoveResult::ERROR:
      ++n_errors;
      break;
    default:
      if (game.get_state().get_winner() == ttt::game::Sign::X)
        ++n_x_wins;
      else if (game.get_state().get_winner() == ttt::game::Sign::O)
        ++n_o_wins;
      else
        ++n_errors;
    }
    game.reset();
  }

  std::cout << "X(mine) wins: " << n_x_wins << "\n"
            << "O(baseline hard) wins: " << n_o_wins << "\n"
            << "draws:  " << n_draws << "\n"
            << "errors: " << n_errors << "\n"
            << std::endl;

  std::cout << "X play time:\n - move time (ms): "
            << tm_p1.get_average_move_time()
            << "\n - event time (ms): " << tm_p1.get_average_event_time()
            << "\n\n";

  std::cout << "O play time:\n - move time (ms): "
            << tm_p2.get_average_move_time()
            << "\n - event time (ms): " << tm_p2.get_average_event_time()
            << "\n\n";

  std::cout << "game process average time: " << game_time_counter.get()
            << " (ms)\n";

  assert(tm_p1.get_average_event_time() < 100);
  assert(tm_p1.get_average_move_time() < 100);

  return 0;
}
