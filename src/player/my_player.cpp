#include "my_player.hpp"
#include "core/state.hpp"
#include <cstdlib>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

int scorePattern(int count, int open_ends) {
  if (count == 5)
    return 1000000;
  if (count == 4 && open_ends == 2)
    return 50000;
  if (count == 4 && open_ends == 1)
    return 1000;
  if (count == 3 && open_ends == 2)
    return 1000;
  if (count == 3 && open_ends == 1)
    return 100;
  if (count == 2 && open_ends == 2)
    return 100;
  if (count == 2 && open_ends == 1)
    return 1;
  return 0;
}

int eval_for_sign(const State &state, const Sign &sgn, int x, int y) {
  int score = 0;
  int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
  for (int d = 0; d < 4; d++) {
    int xn = x;
    int yn = y;
    int dx = directions[d][0];
    int dy = directions[d][1];
    int count = 1;
    int open = 0;
    while (xn + dx >= 0 && yn + dy >= 0 && xn + dx < state.get_opts().rows &&
           yn + dy < state.get_opts().cols &&
           state.get_value(xn + dx, yn + dy) == sgn) {
      xn += dx;
      yn += dy;
      count++;
    }
    if (xn + dx >= 0 && yn + dy >= 0 && xn + dx < state.get_opts().rows &&
        yn + dy < state.get_opts().cols &&
        state.get_value(xn + dx, yn + dy) == Sign::NONE)
      open++;
    xn = x;
    yn = y;
    while (xn - dx >= 0 && yn - dy >= 0 && xn - dx < state.get_opts().rows &&
           yn - dy < state.get_opts().cols &&
           state.get_value(xn - dx, yn - dy) == sgn) {
      xn -= dx;
      yn -= dy;
      count++;
    }
    if (xn - dx >= 0 && yn - dy >= 0 && xn - dx < state.get_opts().rows &&
        yn - dy < state.get_opts().cols &&
        state.get_value(xn - dx, yn - dy) == Sign::NONE)
      open++;
    score += scorePattern(count, open);
  }
  return score;
}

int eval(const State &state, const Sign &sgn) {
  Sign a = Sign::O;
  if (sgn == Sign::O)
    a = Sign::X;
  int score = 0;
  for (int x = 0; x < state.get_opts().rows; x++) {
    for (int y = 0; y < state.get_opts().cols; y++) {
      if (state.get_value(x, y) == sgn)
        score += eval_for_sign(state, sgn, x, y);
      if (state.get_value(x, y) == a)
        score -= eval_for_sign(state, a, x, y) * 2;
    }
  }
  return score;
}

Point MyPlayer::make_move(const State &state) {
  Point result;
  // for (int n_attempt = 0; n_attempt < 50; ++n_attempt) {
  //   result.x = std::rand() % state.get_opts().cols;
  //   result.y = std::rand() % state.get_opts().rows;
  //   if (state.get_value(result.x, result.y) != Sign::NONE) {
  //     --n_attempt;
  //     continue;
  //   }
  //   bool has_neighbors = false;
  //   for (int dx = -1; dx <= 1; ++dx) {
  //     for (int dy = -1; dy <= 1; ++dy) {
  //       if (dx == 0 && dy == 0)
  //         continue;
  //       const Sign val = state.get_value(result.x + dx, result.y + dy);
  //       if (val == Sign::X || val == Sign::O) {
  //         has_neighbors = true;
  //         break;
  //       }
  //     }
  //     if (has_neighbors)
  //       break;
  //   }
  //   if (has_neighbors)
  //     break;
  // }

  return result;
}

}; // namespace ttt::my_player
