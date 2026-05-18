#include "my_player.hpp"
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cmath>
#include <algorithm>

namespace ttt::my_player {

    static const int CANDIDATE_RADIUS = 4;
    static const int MAX_CANDIDATES = 50;
    static const int INNER_MAX_CANDIDATES = 20;
    static const int INNER_RADIUS = 2;

    static const struct {
        int dx;
        int dy;
    } directions[] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };

    static bool on_field(int x, int y) {
        return x >= 0 && x < 20 && y >= 0 && y < 20;
    }

    // Легковесное состояние для быстрой симуляции
    struct LightState {
        Sign grid[20][20];
        Sign current_player;
        int move_no;

        void from_state(const State& s) {
            for (int y = 0; y < 20; ++y)
                for (int x = 0; x < 20; ++x)
                    grid[y][x] = s.get_value(x, y);
            current_player = s.get_current_player();
            move_no = s.get_move_no();
        }

        Sign get(int x, int y) const {
            if (!on_field(x, y)) return Sign::WALL;
            return grid[y][x];
        }

        void apply_move(int x, int y) {
            grid[y][x] = current_player;
            current_player = (current_player == Sign::X) ? Sign::O : Sign::X;
            move_no++;
        }
    };

    // Оценка одной линии из 5 клеток (из Заданий 1-3)
    static int evaluate_line_light(int my_count, int enemy_count) {
        if (my_count > 0 && enemy_count > 0) return 0;
        if (my_count == 0 && enemy_count == 0) return 0;

        if (my_count > 0) {
            if (my_count >= 5) return 10000000;
            if (my_count == 4) return 50000;
            if (my_count == 3) return 1000;
            if (my_count == 2) return 100;
            return 10;
        } else {
            if (enemy_count >= 5) return -10000000;
            if (enemy_count == 4) return -50000;
            if (enemy_count == 3) return -1000;
            if (enemy_count == 2) return -100;
            return -10;
        }
    }

    // Быстрая оценка всей доски (из Заданий 1-3)
    static int score_all_light(const LightState& state, Sign my_sign) {
        int total_score = 0;
        Sign enemy_sign = (my_sign == Sign::X) ? Sign::O : Sign::X;

        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                for (const auto& dir : directions) {
                    int my_count = 0;
                    int enemy_count = 0;
                    bool valid = true;

                    for (int i = 0; i < 5; ++i) {
                        int nx = x + dir.dx * i;
                        int ny = y + dir.dy * i;
                        Sign s = state.get(nx, ny);

                        if (s == Sign::WALL) {
                            valid = false;
                            break;
                        }
                        if (s == my_sign) my_count++;
                        else if (s == enemy_sign) enemy_count++;
                    }

                    if (valid) {
                        total_score += evaluate_line_light(my_count, enemy_count);
                    }
                }
            }
        }
        return total_score;
    }

    // Проверка на победную комбинацию на легком состоянии
    static bool is_winning_light(const LightState& state, Sign player) {
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                if (state.grid[y][x] != player) continue;
                for (const auto& dir : directions) {
                    int count = 1;
                    for (int i = 1; i < 5; ++i) {
                        if (state.get(x + dir.dx * i, y + dir.dy * i) == player) count++;
                        else break;
                    }
                    if (count >= 5) return true;
                }
            }
        }
        return false;
    }

    // Поиск перспективных клеток вокруг существующих фигур
    static int get_candidates_light(const LightState& state, Sign active_player, Point* out_pts, int max_cand, int radius) {
        bool visited[20][20] = { false };
        int count = 0;

        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                if (state.grid[y][x] == Sign::X || state.grid[y][x] == Sign::O) {
                    for (int dy = -radius; dy <= radius; ++dy) {
                        for (int dx = -radius; dx <= radius; ++dx) {
                            int nx = x + dx;
                            int ny = y + dy;
                            if (on_field(nx, ny) && state.grid[ny][nx] == Sign::NONE && !visited[ny][nx]) {
                                visited[ny][nx] = true;
                                out_pts[count++] = Point{ nx, ny };
                                if (count >= max_cand) return count;
                            }
                        }
                    }
                }
            }
        }

        if (count == 0 && state.grid[10][10] == Sign::NONE) {
            out_pts[count++] = Point{ 10, 10 };
        }
        return count;
    }

    // ЗАДАНIE 4: Классический алгоритм Минимакс без альфа-бета отсечения
    static int minimax(const LightState& state, int depth, bool is_maximizing, Sign my_sign, clock_t deadline, int& nodes_accessed) {
        nodes_accessed++;

        // Базовый случай: достигли максимальной глубины симуляции или вышло время
        if (depth == 0 || clock() > deadline) {
            return score_all_light(state, my_sign);
        }

        Sign opp_sign = (my_sign == Sign::X) ? Sign::O : Sign::X;
        Point candidates[INNER_MAX_CANDIDATES];
        
        // Генерируем ходы для текущего активного игрока в дереве
        Sign active_sign = is_maximizing ? my_sign : opp_sign;
        int cand_count = get_candidates_light(state, active_sign, candidates, INNER_MAX_CANDIDATES, INNER_RADIUS);

        if (cand_count == 0) {
            return score_all_light(state, my_sign);
        }

        if (is_maximizing) {
            int max_score = -1000000000;
            for (int i = 0; i < cand_count; ++i) {
                LightState child = state;
                child.apply_move(candidates[i].x, candidates[i].y);

                int score = minimax(child, depth - 1, false, my_sign, deadline, nodes_accessed);
                
                if (score > max_score) {
                    max_score = score;
                }
                // Альфа-бета отсечение убрано, цикл всегда доходит до конца
            }
            return max_score;
        } else {
            int min_score = 1000000000;
            for (int i = 0; i < cand_count; ++i) {
                LightState child = state;
                child.apply_move(candidates[i].x, candidates[i].y);

                int score = minimax(child, depth - 1, true, my_sign, deadline, nodes_accessed);
                
                if (score < min_score) {
                    min_score = score;
                }
                // Альфа-бета отсечение убрано, цикл всегда доходит до конца
            }
            return min_score;
        }
    }

    void MyPlayer::set_sign(Sign sign) {
        m_sign = sign;
    }

    const char* MyPlayer::get_name() const {
        return m_name;
    }

    // Главный метод принятия решения ходов
    Point MyPlayer::make_move(const State& game) {
        LightState root;
        root.from_state(game);

        Point candidates[MAX_CANDIDATES];
        int cand_count = get_candidates_light(root, m_sign, candidates, MAX_CANDIDATES, CANDIDATE_RADIUS);

        if (cand_count == 0) return Point{ 10, 10 };

        Sign opp_sign = (m_sign == Sign::X) ? Sign::O : Sign::X;

        // 1. Мгновенная атака: если можем победить в один ход — ходим туда сразу
        for (int i = 0; i < cand_count; ++i) {
            LightState test = root;
            test.apply_move(candidates[i].x, candidates[i].y);
            if (is_winning_light(test, m_sign)) {
                return candidates[i];
            }
        }

        // 2. Мгновенная защита: если соперник может победить в один ход — блокируем
        for (int i = 0; i < cand_count; ++i) {
            LightState test = root;
            test.current_player = opp_sign; 
            test.apply_move(candidates[i].x, candidates[i].y);
            if (is_winning_light(test, opp_sign)) {
                return candidates[i];
            }
        }

        // 3. Итеративное углубление с чистым Минимаксом
        clock_t start_time = clock();
        clock_t deadline = start_time + (CLOCKS_PER_SEC * 80 / 1000); // 80 мс лимит на ход

        Point best_move = candidates[0];
        int best_score = -1000000000;

        // Идем по глубинам, исключая альфа-бета параметры из вызова
        for (int depth = 2; depth <= 4; ++depth) {
            int current_best_score = -1000000000;
            Point current_best = candidates[0];
            bool time_out = false;

            for (int i = 0; i < cand_count; ++i) {
                LightState child = root;
                child.apply_move(candidates[i].x, candidates[i].y);

                if (score_all_light(child, m_sign) >= 10000000) {
                    return candidates[i];
                }

                int dummy_nodes = 0;
                // ВЫЗОВ: без параметров alpha и beta
                int score = minimax(child, depth - 1, false, m_sign, deadline, dummy_nodes);
                
                if (clock() > deadline) {
                    time_out = true;
                    break;
                }
                
                if (score > current_best_score) {
                    current_best_score = score;
                    current_best = candidates[i];
                }
            }

            if (!time_out) {
                best_score = current_best_score;
                best_move = current_best;
            } else {
                break; // если вышли за рамки времени, оставляем результат предыдущей глубины
            }
        }

        return best_move;
    }

} // namespace ttt::my_player