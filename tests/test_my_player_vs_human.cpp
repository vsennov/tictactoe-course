#include "player/my_observer.hpp"
#include "player/my_player.hpp"
#include "human_player.hpp"
#include "core/baseline.hpp"



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
  opts.rows = opts.cols = 15; 
  opts.win_len = 5; 
  opts.max_moves = 0;

  ttt::my_player::MyPlayer p1("MyAIPlayer"); // <~~ put your ai player here
  ttt::human_player::HumanPlayer p2("human_player");
    // ttt::game::IPlayer *p1 = ttt::baseline::get_harder_player("p_easy"); <~~ вы также можете играть против базовых игроков
    // ttt::game::IPlayer *p1 = ttt::baseline::get_easy_player("p_easy"); <~~ вы также можете играть против базовых игроков
    // ttt::my_player::MyPlayer prand("prand");
  ttt::my_player::ConsoleWriter obs;
  ttt::game::Game game(opts);
  game.add_player(ttt::game::Sign::X, &p1);// если вы тестируете против базовых игроков, просто передайте указатель p1

  game.add_player(ttt::game::Sign::O, &p2); 
  game.add_observer(&obs);

  std::cout << "\n";
  while (game.process() == ttt::game::MoveResult::OK) {

    obs.print_game_state(game.get_state());
  }

}
