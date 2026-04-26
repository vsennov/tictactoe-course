#include "my_player.hpp"
#include "core/state.hpp"
#include <cstdlib>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

void build_line(const State &state, Sign sgn, int x, int y, int dx, int dy,
                int line[9]) {
  for (int k = -4; k <= 4; k++) {
    int nx = x + k * dx;
    int ny = y + k * dy;

    int idx = k + 4;

    if (nx < 0 || ny < 0 || nx >= state.get_opts().rows ||
        ny >= state.get_opts().cols) {
      line[idx] = 3;
    } else {
      Sign v = state.get_value(nx, ny);
      if (v == sgn)
        line[idx] = 1;
      else if (v == Sign::NONE)
        line[idx] = 0;
      else
        line[idx] = 2;
    }
  }
}

int score_line(int line[9]) {
  int score = 0;

  for (int i = 0; i <= 4; i++) {
    // XXXXX
    if (line[i] == 1 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 1)
      score += 30;

    // XX.XX
    if (line[i] == 1 && line[i + 1] == 1 && line[i + 2] == 0 &&
        line[i + 3] == 1 && line[i + 4] == 1)
      score += 5;

    // X.XXX
    if (line[i] == 1 && line[i + 1] == 0 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 1)
      score += 5;

    // XXX.X
    if (line[i] == 1 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 0 && line[i + 4] == 1)
      score += 5;

    // XXXX.
    if (line[i] == 1 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 0)
      score += 5;

    // .XXXX
    if (line[i] == 0 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 1)
      score += 5;
  }

  for (int i = 0; i <= 3; i++) {
    // .XX.X.
    if (line[i] == 0 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 0 && line[i + 4] == 1 && line[i + 5] == 0)
      score += 3;

    // .X.XX.
    if (line[i] == 0 && line[i + 1] == 1 && line[i + 2] == 0 &&
        line[i + 3] == 1 && line[i + 4] == 1 && line[i + 5] == 0)
      score += 3;

    // ..XX..
    if (line[i] == 0 && line[i + 1] == 0 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 0 && line[i + 5] == 0)
      score += 2;

    // .X..X.
    if (line[i] == 0 && line[i + 1] == 1 && line[i + 2] == 0 &&
        line[i + 3] == 0 && line[i + 4] == 1 && line[i + 5] == 0)
      score += 1;
  }

  for (int i = 0; i <= 2; i++) {
    // ..XXX..
    if (line[i] == 0 && line[i + 1] == 0 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 1 && line[i + 5] == 0 &&
        line[i + 6] == 0)
      score += 3;
    // ..X.X..
    if (line[i] == 0 && line[i + 1] == 0 && line[i + 2] == 1 &&
        line[i + 3] == 0 && line[i + 4] == 1 && line[i + 5] == 0 &&
        line[i + 6] == 0)
      score += 2;
  }
  return score;
}

int scoreBonusPattern(const State &state, Sign sgn, int x, int y) {
  int score = 0;
  int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

  for (int d = 0; d < 4; d++) {
    int dx = directions[d][0];
    int dy = directions[d][1];
    int line[9];
    build_line(state, sgn, x, y, dx, dy, line);
    score += score_line(line);
  }

  return score;
}

int scorePattern(int count, int open_ends) {
  if (count == 5)
    return 30;
  if (count == 4 && open_ends == 2)
    return 8;
  if (count == 4 && open_ends == 1)
    return 5;
  if (count == 3 && open_ends == 2)
    return 3;
  if (count == 3 && open_ends == 1)
    return 2;
  if (count == 2 && open_ends == 2)
    return 2;
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
    if (x - dx >= 0 && y - dy >= 0 && x - dx < state.get_opts().rows &&
        y - dy < state.get_opts().cols &&
        state.get_value(x - dx, y - dy) == sgn)
      continue;
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

double eval(const State &state, const Sign &sgn) {
  Sign a = Sign::O;
  if (sgn == Sign::O)
    a = Sign::X;
  double score = 0.0;
  for (int x = 0; x < state.get_opts().rows; x++) {
    for (int y = 0; y < state.get_opts().cols; y++) {
      if (state.get_value(x, y) == sgn) {
        score += eval_for_sign(state, sgn, x, y);
        score += scoreBonusPattern(state, sgn, x, y);
      }
      if (state.get_value(x, y) == a) {
        score -= eval_for_sign(state, a, x, y) * 1.1;
        score -= scoreBonusPattern(state, a, x, y);
      }
    }
  }
  return score;
}

Point find_start_move(const State &state) {
  int rows = state.get_opts().rows;
  int cols = state.get_opts().cols;
  Point p;
  int cx = rows / 2;
  int cy = cols / 2;

  int max_r = (rows > cols) ? rows : cols;
  for (int r = 0; r <= max_r; r++) {
    for (int dx = -r; dx <= r; dx++) {
      for (int dy = -r; dy <= r; dy++) {
        if (abs(dx) != r && abs(dy) != r)
          continue;

        int x = cx + dx;
        int y = cy + dy;

        if (x < 0 || y < 0 || x >= rows || y >= cols)
          continue;
        if (state.get_value(x, y) == Sign::NONE) {
          p.x = x;
          p.y = y;
          return p;
        }
      }
    }
  }
  p.x = 0;
  p.y = 0;
  return p;
}

Point MyPlayer::make_move(const State &state) {
  Point best_move;
  double best_score = -10000000;
  Sign opponent = (m_sign == Sign::X ? Sign::O : Sign::X);

  if (state.get_move_no() == 0)
    return find_start_move(state);

  for (int x = 0; x < state.get_opts().rows; x++) {
    for (int y = 0; y < state.get_opts().cols; y++) {
      if (state.get_value(x, y) != Sign::NONE)
        continue;

      bool has_neighbors = false;
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
          if (dx == 0 && dy == 0)
            continue;

          int nx = x + dx;
          int ny = y + dy;

          if (nx >= 0 && ny >= 0 && nx < state.get_opts().rows &&
              ny < state.get_opts().cols) {

            if (state.get_value(nx, ny) != Sign::NONE) {
              has_neighbors = true;
            }
          }
        }
      }
      if (!has_neighbors)
        continue;

      State new_state = state;
      new_state.process_move(m_sign, x, y);
      double score = eval(new_state, m_sign);
      if (score > best_score) {
        best_score = score;
        best_move.x = x;
        best_move.y = y;
      }
    }
  }

  return best_move;

  // Point result;
  //  for (int n_attempt = 0; n_attempt < 50; ++n_attempt) {
  //    result.x = std::rand() % state.get_opts().cols;
  //    result.y = std::rand() % state.get_opts().rows;
  //    if (state.get_value(result.x, result.y) != Sign::NONE) {
  //      --n_attempt;
  //      continue;
  //    }
  //    bool has_neighbors = false;
  //    for (int dx = -1; dx <= 1; ++dx) {
  //      for (int dy = -1; dy <= 1; ++dy) {
  //        if (dx == 0 && dy == 0)
  //          continue;
  //        const Sign val = state.get_value(result.x + dx, result.y + dy);
  //        if (val == Sign::X || val == Sign::O) {
  //          has_neighbors = true;
  //          break;
  //        }
  //      }
  //      if (has_neighbors)
  //        break;
  //    }
  //    if (has_neighbors)
  //      break;
  //  }
  //  return result;
}

}; // namespace ttt::my_player
