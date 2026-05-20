#include "my_player.hpp"
#include <cstdlib>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

MoveStats MyPlayer::evaluate_move(const State& state, int cx, int cy, Sign color) const {
        MoveStats s;
        int win_len = state.get_opts().win_len;
        int w = state.get_opts().cols;
        int h = state.get_opts().rows;

        int dx[4] = { 1, 0, 1, 1 };
        int dy[4] = { 0, 1, 1, -1 };
        //направления: →  ↓  ↘  ↗  

        for (int d = 0; d < 4; d++) {
            int d_fours = 0;
            int d_threes = 0;
            int d_twos = 0;

            for (int offset = -win_len + 1; offset <= 0; offset++) {
                int start_x = cx + offset * dx[d];
                int start_y = cy + offset * dy[d];

                bool ok = true;//ограничения ходов на поле, чтоб за поле не выходил и на чужую клетку
                int mine = 0; // кол камней в линии


                //подсчет mine и блокировок
                for (int i = 0; i < win_len; i++) {
                    int px = start_x + i * dx[d];
                    int py = start_y + i * dy[d];

                    if (px < 0 || px >= w || py < 0 || py >= h) {
                        ok = false;
                        break;
                    }

                    Sign cell = state.get_value(px, py);
                    if (px == cx && py == cy) {
                        mine++;
                    }
                    else if (cell == color) {
                        mine++;
                    }
                    else if (cell != Sign::NONE) {
                        ok = false;
                        break;
                    }
                }
                //Классификация результата
                if (ok) {
                    if (mine == win_len) {
                        s.wins++;//победа
                    }
                    else if (mine == win_len - 1) {
                        d_fours++;// четвёрка
                    }
                    else if (mine == win_len - 2) {
                        d_threes++;//тройка
                    }
                    else if (mine == win_len - 3) {
                        d_twos++;//двойка
                    }
                }
            }

            if (d_fours >= 2) {
                s.open_fours++;
            }
            else if (d_fours == 1) {
                s.fours++;
            }

            if (d_threes >= 2) {
                s.open_threes++;
            }
            else if (d_threes == 1) {
                s.threes++;
            }

            s.twos += d_twos;
        }
        return s;
    }

bool MyPlayer::has_neighbor(const State& state, int cx, int cy, int radius) const {
        int w = state.get_opts().cols;
        int h = state.get_opts().rows;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = cx + dx;
                int ny = cy + dy;
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    Sign val = state.get_value(nx, ny);
                    if (val == Sign::X || val == Sign::O) {
                        return true;
                    }
                }
            }
        }
        return false;
    }


Point MyPlayer::make_move(const State &state) {
  Point result;
  for (int n_attempt = 0; n_attempt < 50; ++n_attempt) {
    result.x = std::rand() % state.get_opts().cols;
    result.y = std::rand() % state.get_opts().rows;
    if (state.get_value(result.x, result.y) != Sign::NONE) {
      --n_attempt;
      continue;
    }
    bool has_neighbors = false;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        if (dx == 0 && dy == 0)
          continue;
        const Sign val = state.get_value(result.x + dx, result.y + dy);
        if (val == Sign::X || val == Sign::O) {
          has_neighbors = true;
          break;
        }
      }
      if (has_neighbors)
        break;
    }
    if (has_neighbors)
      break;
  }
  return result;
}


}; // namespace ttt::my_player