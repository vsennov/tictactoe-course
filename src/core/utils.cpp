#include "utils.hpp"
#include <iomanip>

namespace ttt::utils {

void print_game_state(const ttt::game::State& state) {
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;
  
  //print column indices
  std::cout << "   "; // extra space for column index
  for (int x = 0; x < cols; ++x) {
    std::cout << std::setw(2) << x%10;
  }
  std::cout << "\n";
  
  //line separator
  std::cout << "   +";
  for (int x = 0; x < cols; ++x) {
    std::cout << "--";
  }
  std::cout << "\n";
  
  //print board with row indices
  for (int y = 0; y < rows; ++y) {
    std::cout << std::setw(2) << y << " |";
    for (int x = 0; x < cols; ++x) {
      char c = '.';
      switch (state.get_value(x, y)) {
        case ttt::game::Sign::X:
          c = 'X';
          break;
        case ttt::game::Sign::O:
          c = 'O';
          break;
        default:
          break;
      }
      std::cout << c << " ";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

} // namespace ttt::utils