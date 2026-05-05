#include "my_player.hpp"
#include "core/state.hpp"
#include <cstdlib>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

struct FastBoard {
  Sign grid[20][20];
  int rows;
  int cols;
  void sync(const State &state) {
    rows = state.get_opts().rows;
    cols = state.get_opts().cols;
    for (int x = 0; x < rows; x++) {
      for (int y = 0; y < cols; y++) {
        grid[x][y] = state.get_value(x, y);
      }
    }
  }
  void make_move(int x, int y, Sign sgn) { grid[x][y] = sgn; }
  void undo_move(int x, int y) { grid[x][y] = Sign::NONE; }
  Sign get(int x, int y) const {
    if (x < 0 || y < 0 || x >= rows || y >= cols)
      return Sign::WALL;
    return grid[x][y];
  }
};

bool is_promising(const FastBoard &fb, int x, int y) {
  for (int dx = -2; dx <= 2; dx++) {
    for (int dy = -2; dy <= 2; dy++) {
      if (dx == 0 && dy == 0)
        continue;
      Sign val = fb.get(x + dx, y + dy);
      if (val == Sign::X || val == Sign::O)
        return true;
    }
  }
  return false;
}

void build_line(const FastBoard &fb, Sign sgn, int x, int y, int dx, int dy,
                int line[9]) {
  for (int k = -4; k <= 4; k++) {
    int idx = k + 4;
    if (k == 0) {
      line[idx] = 1;
      continue;
    }

    Sign v = fb.get(x + k * dx, y + k * dy);
    if (v == sgn)
      line[idx] = 1;
    else if (v == Sign::NONE)
      line[idx] = 0;
    else
      line[idx] = 2;
  }
}

int get_index(const int window[5]) {
  int index = 0;
  int power = 1;
  for (int i = 0; i < 5; i++) {
    index += window[i] * power;
    power *= 3;
  }
  return index;
}

static int pointTable[243];

void init_lookup_table() {
  for (int i = 0; i < 243; i++) {
    int temp = i;
    int window[5];
    int stones = 0;
    int blocked = 0;

    for (int j = 0; j < 5; j++) {
      window[j] = temp % 3;
      if (window[j] == 1)
        stones++;
      if (window[j] == 2)
        blocked++;
      temp /= 3;
    }
    if (blocked > 0) {
      pointTable[i] = 0;
    } else {
      if (stones == 5)
        pointTable[i] = 100000000;
      else if (stones == 4) {
        pointTable[i] = 20000;
      } else if (stones == 3) {
        if (window[0] == 0 && window[4] == 0)
          pointTable[i] = 100000;
        else
          pointTable[i] = 5000;
        // pointTable[i] = 1000;
      } else if (stones == 2)
        if (window[0] == 0 && window[4] == 0)
          pointTable[i] = 10000; // Открытая двойка (0110) — это база для тройки
        else
          pointTable[i] = 1000;
      else
        pointTable[i] = 0;
    }
  }
}

int score_line_segment(int line[9]) {
  int score = 0;
  for (int i = 0; i <= 4; i++) {
    int index = 0;
    int power = 1;
    for (int j = 0; j < 5; j++) {
      index += line[i + j] * power;
      power *= 3;
    }
    score += pointTable[index];
  }

  for (int i = 0; i <= 3; i++) {
    if (line[i] == 0 && line[i + 1] == 1 && line[i + 2] == 1 &&
        line[i + 3] == 1 && line[i + 4] == 1 && line[i + 5] == 0)
      score += 1000000;
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

int attack_score(const FastBoard &fb, Sign sgn, int x, int y) {
  int score = 0;
  int threats_count = 0;
  int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
  for (auto &d : directions) {
    int line[9];
    build_line(fb, sgn, x, y, d[0], d[1], line);
    // score += score_line_segment(line);
    int s = score_line_segment(line);
    if (s >= 1000)
      threats_count++;
    score += s;
  }
  if (threats_count >= 2) {
    score *= 10;
  }
  return score;
}

int evaluate_board(const FastBoard &fb, Sign current_sign) {
  int my_score = 0;
  int opp_score = 0;
  Sign opponent = (current_sign == Sign::X) ? Sign::O : Sign::X;

  for (int x = 0; x < fb.rows; x++) {
    for (int y = 0; y < fb.cols; y++) {
      // Оцениваем только клетки рядом с камнями
      if (fb.get(x, y) == Sign::NONE && is_promising(fb, x, y)) {
        my_score += attack_score(fb, current_sign, x, y);
        opp_score += attack_score(fb, opponent, x, y);
      }
    }
  }
  // Возвращаем разницу. Это заставит бота учитывать ВСЕ угрозы на доске.
  return my_score - (int)(opp_score * 0.9);
}

// без альфа бета
int negamax(FastBoard &fb, int depth, Sign current_sign, Sign bot_sign) {
  if (depth == 0) {
    int score = evaluate_board(fb, bot_sign);
    return (current_sign == bot_sign) ? score : -score;
  }
  int max_score = -100000000;
  Sign opponent = (current_sign == Sign::X) ? Sign::O : Sign::X;
  for (int x = 0; x < fb.rows; x++) {
    for (int y = 0; y < fb.cols; y++) {
      if (fb.get(x, y) != Sign::NONE || !is_promising(fb, x, y))
        continue;
      fb.make_move(x, y, current_sign);
      int score = -negamax(fb, depth - 1, opponent, bot_sign);
      fb.undo_move(x, y);
      if (score > max_score)
        max_score = score;
    }
  }
  return max_score;
}

Point MyPlayer::make_move(const State &state) {
  init_lookup_table();
  if (state.get_move_no() == 0)
    return find_start_move(state);
  FastBoard fb;
  fb.sync(state); // Копируем данные в наш быстрый массив ОДИН раз

  Point best_move = {0, 0};
  double max_weight = -1e9;
  Sign opponent = (m_sign == Sign::X ? Sign::O : Sign::X);

  for (int x = 0; x < fb.rows; x++) {
    for (int y = 0; y < fb.cols; y++) {
      if (fb.get(x, y) != Sign::NONE || !is_promising(fb, x, y))
        continue;

      // 1. Делаем пробный ход
      fb.make_move(x, y, m_sign);

      // 2. Оцениваем последствия этого хода через негамакс
      // Мы передаем opponent, так как следующий ход за ним
      int score = -negamax(fb, 1, opponent, m_sign);

      // 3. Отменяем ход
      fb.undo_move(x, y);

      if (score > max_weight) {
        max_weight = score;
        best_move = {x, y};
      }
    }
  }

  return best_move;
}

}; // namespace ttt::my_player
