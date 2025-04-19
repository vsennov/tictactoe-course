#include "player/my_observer.hpp"
#include "player/my_player.hpp"
#include "player/human_player.hpp"
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
  opts.rows = opts.cols = 15; // <~~ adjust the size of the board here
  opts.win_len = 5; // <~~ adjust the win_length here
  opts.max_moves = 0;

  ttt::my_player::MyPlayer p1("MyAIPlayer"); // <~~ put your ai player here
  ttt::human_player::HumanPlayer p2("human_player");
    // ttt::game::IPlayer *p1 = ttt::baseline::get_harder_player("p_easy"); <~~ you can also test against the baseline players
    // ttt::game::IPlayer *p1 = ttt::baseline::get_easy_player("p_easy"); <~~ you can also test against the baseline players
    // ttt::my_player::MyPlayer prand("prand");
  ttt::my_player::ConsoleWriter obs;
  ttt::game::Game game(opts);
  game.add_player(ttt::game::Sign::X, &p1);// if you test against the baseline players, just pass the p1 pointer
  game.add_player(ttt::game::Sign::O, &p2); 
  game.add_observer(&obs);

  std::cout << "\n";
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

  //  delete p1; <~~ don't forget to delete the pointer (baseline case)
}
