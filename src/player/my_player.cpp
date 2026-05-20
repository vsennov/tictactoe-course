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
  int w = state.get_opts().cols;
        int h = state.get_opts().rows;
        Sign opp;

        if (m_sign == Sign::X) {
            opp = Sign::O;
        }
        else {
            opp = Sign::X;
        }

        int best_prio = 8; // Наивысший приоритет
        double best_score = -1.0; // минимальный бал
        Point best_move = { 0, 0 };
        bool found_any = false; // найден ли хоть один активный ход

        //лучший ход
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                if (state.get_value(x, y) != Sign::NONE) continue; // Пропуск занятых клеток

                
                if (!has_neighbor(state, x, y, 2)) continue;
                found_any = true; 

                
                MoveStats att = evaluate_move(state, x, y, m_sign); // Оценка атаки
                MoveStats def = evaluate_move(state, x, y, opp);   // Оценка защиты


                int prio = 7;
                if (att.wins > 0) {
                    prio = 1;
                }
                else if (def.wins > 0) {
                    prio = 2;
                }
                else if (att.open_fours > 0) {
                    prio = 3;
                }
                else if (def.open_fours > 0) {
                    prio = 4;
                }
                else if ((att.open_fours + att.fours + att.open_threes) >= 2) {
                    prio = 5;
                }
                else if ((def.open_fours + def.fours + def.open_threes) >= 2) {
                    prio = 6;
                }
                


                //Расчёт числового балла (score)
                double score_att = att.fours * 1000 + att.open_threes * 500 + att.threes * 100 + att.twos * 10;
                double score_def = def.fours * 1000 + def.open_threes * 500 + def.threes * 100 + def.twos * 10;

                //штраф если ход далеко от центра
                double dx = x - w / 2.0;
                double dy = y - h / 2.0;
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;

                double score = (score_att * 1.1) + score_def - (dx + dy); // Итоговая формула оценки

                //Выбор лучшего хода
                if (prio < best_prio) { 
                    best_prio = prio;
                    best_score = score;
                    best_move = { x, y };
                }
                else if (prio == best_prio) { 
                    if (score > best_score) {
                        best_score = score;
                        best_move = { x, y };
                    }
                }
            }
        }
        if (!found_any) {
            int cx = w / 2;
            int cy = h / 2;
            int min_d = 999999;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    if (state.get_value(x, y) == Sign::NONE) {
                        int d_x = x - cx;
                        if (d_x < 0) d_x = -d_x;

                        int d_y = y - cy;
                        if (d_y < 0) d_y = -d_y;

                        if (d_x + d_y < min_d) {
                            min_d = d_x + d_y;
                            best_move = { x, y };
                        }
                    }
                }
            }
        }

        return best_move;
    }
}; // namespace ttt::my_player