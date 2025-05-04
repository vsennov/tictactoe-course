#include "core/baseline.hpp"
#include "player/my_observer.hpp"
#include "player/my_player.hpp"
#include "core/game.hpp"


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    std::srand(atoi(argv[1]));
  }

  ttt::game::State::Opts opts;
  opts.rows = opts.cols = 15;
  opts.win_len = 5;
  opts.max_moves = 0;

  ttt::game::IPlayer *p1 = new ttt::my_player::MyPlayer("MyPlayer");  // <~~ add your player here
  ttt::game::IPlayer *p2 = ttt::baseline::get_harder_player("p_easy");

  ttt::my_player::MyPlayer prand("prand");

  ttt::my_player::ConsoleWriter obs;

  ttt::game::Game game(opts);
  game.add_player(ttt::game::Sign::X, p1);
  game.add_player(ttt::game::Sign::O, p2);
  game.add_observer(&obs);

  
  while (game.process() == ttt::game::MoveResult::OK) {
  
    obs.print_game_state(game.get_state());
    
    
  }

  delete p1;
  delete p2;
}
