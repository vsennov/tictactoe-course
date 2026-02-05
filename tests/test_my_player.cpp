#include "player/my_observer.hpp"
#include "player/my_player.hpp"


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>

int main(int argc, char *argv[]) {
  std::cout << "Hello!\n";
  if (argc >= 2) {
    std::srand(atoi(argv[1]));
  }

  ttt::game::State::Opts opts;
  opts.rows = opts.cols = 20;
  opts.win_len = 20;
  opts.max_moves = 0;

  auto field_initializer = ttt::game::RandomObstaclesFI(0.75, 50, 1);

  ttt::my_player::MyPlayer p1("p1");
  ttt::my_player::MyPlayer p2("p2");
  ttt::my_player::ConsoleWriter obs;

  ttt::game::Game game(opts, &field_initializer);
  game.add_player(ttt::game::Sign::X, &p1);
  game.add_player(ttt::game::Sign::O, &p2);
  game.add_observer(&obs);

  obs.print_game_state(game.get_state());
  while (game.process() == ttt::game::MoveResult::OK) {
    obs.print_game_state(game.get_state());
  }
  obs.print_game_state(game.get_state());
}
