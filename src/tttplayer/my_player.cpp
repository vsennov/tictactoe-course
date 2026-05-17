#include "my_player.hpp"
#include "line_scan.hpp"
#include <algorithm>
#include <iostream>
#include <limits>

namespace ttt::my_player {

int evaluate_line(const ttt::an::LineStats& s) {
    if (s.my_cnt >= 4) return 1000000; // Победа
    if (s.opp_cnt >= 4) return 500000; // Срочная блокировка

    int score = 0;
    if (s.my_cnt == 3 && !s.front_blocked && !s.back_blocked) score += 10000;
    if (s.opp_cnt == 3 && !s.front_blocked && !s.back_blocked) score += 8000;
    
    score += s.my_cnt * 100;
    return score;
}

ttt::game::Point MyPlayer::make_move(const ttt::game::State& state) {
    const auto& opts = state.get_opts();
    ttt::game::Point best_move{0, 0};
    long long max_score = -2000000000LL; // Очень маленькое число

    for (int y = 0; y < opts.rows; ++y) {
        for (int x = 0; x < opts.cols; ++x) {
            if (state.get_value(x, y) == ttt::game::Sign::NONE) {
                
                auto directions = ttt::an::scan_all_dirs(state, x, y, sign_);
                long long cell_score = 0;

                for (const auto& dir_stats : directions) {
                    cell_score += evaluate_line(dir_stats);
                }

                // ЖЕСТКИЙ ПРИОРИТЕТ ЦЕНТРА
                // Вычисляем расстояние от центра (чем меньше, тем лучше)
                int dist = std::abs(x - opts.cols / 2) + std::abs(y - opts.rows / 2);
                
                // Штрафуем за удаление от центра. 
                // В центре dist=0, штраф 0. В углу (4,4) dist=4, штраф -40.
                cell_score -= (dist * 10); 

                if (cell_score > max_score) {
                    max_score = cell_score;
                    best_move = {x, y};
                }
            }
        }
    }

    // Раскомментируйте строку ниже, если хотите видеть координаты в консоли:
    // std::cerr << "Bot chooses: (" << best_move.x << "," << best_move.y << ") with score " << max_score << std::endl;

    return best_move;
}

} // namespace ttt::my_player
