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

  // auto p1 = new ttt::my_player::MyPlayer("MyAIPlayer");
  // auto p1 = ttt::baseline::get_harder_player("p_easy");
  auto p1 = ttt::baseline::get_easy_player("p_easy");
  auto p2 = new ttt::human_player::HumanPlayer("human_player");
  ttt::my_player::ConsoleWriter obs;
  ttt::game::Game game(opts);
  game.add_player(ttt::game::Sign::X, p1);// если вы тестируете против базовых игроков, просто передайте указатель p1

  game.add_player(ttt::game::Sign::O, p2); 
  game.add_observer(&obs);

  std::cout << "\n";
  while (game.process() == ttt::game::MoveResult::OK) {}
  delete p1;
  delete p2;
}
