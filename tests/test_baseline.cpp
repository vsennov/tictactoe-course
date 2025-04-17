#include "core/baseline.hpp"
#include "player/my_observer.hpp"
#include "player/my_player.hpp"


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
  // game.add_player(ttt::game::Sign::O, &prand);
  game.add_observer(&obs);

  
  while (game.process() == ttt::game::MoveResult::OK) {
    // Print the current state of the board
    std::cout << "   "; //extra space for column index
    for (int x = 0; x < opts.cols; ++x) {
      std::cout << std::setw(2) << x%10;
    }
    std::cout << "\n";
    
    //line separator
    std::cout << "   +";
    for (int x = 0; x < opts.cols; ++x) {
      std::cout << "--";
    }
    std::cout << "\n";
    
    // print board with row indexes
    for (int y = 0; y < opts.rows; ++y) {
      std::cout << std::setw(2) << y << " |";
      for (int x = 0; x < opts.cols; ++x) {
        char c = '.';
        switch (game.get_state().get_value(x, y)) {
        case ttt::game::Sign::X:
          c = 'X';
          break;
        case ttt::game::Sign::O:
          c = 'O';
          break;
        default:
          break;
        }
        std::cout << c << " "; //add spaces for readability and better debugging
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  delete p1;
  delete p2;
}
