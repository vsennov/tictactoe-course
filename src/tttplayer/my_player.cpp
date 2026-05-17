#include "tttplayer/my_player.hpp"
#include "tttplayer/line_scan.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ttt::my_player {

int evaluate_line(const ttt::an::LineStats& s) {
    if (s.my_cnt >= 4) return 1000000;
    if (s.opp_cnt >= 4) return 500000;

    int score = 0;

    if (s.my_cnt == 3) {
        if (!s.front_blocked && !s.back_blocked) score += 10000;
        else if (!s.front_blocked || !s.back_blocked) score += 1000;
    }

    if (s.opp_cnt == 3) {
        if (!s.front_blocked && !s.back_blocked) score += 8000;
        else score += 500;
    }

    score += s.my_cnt * 100;
    if (s.front_open) score += 10;
    if (s.back_open) score += 10;

    return score;
}

ttt::game::Point MyPlayer::make_move(const ttt::game::State& state) {
    const auto& opts = state.get_opts();
    ttt::game::Point best_move{0, 0};
    long long max_score = std::numeric_limits<long long>::min();

    for (int y = 0; y < opts.rows; ++y) {
        for (int x = 0; x < opts.cols; ++x) {
            if (state.get_value(x, y) == ttt::game::Sign::NONE) {
                
                auto directions = ttt::an::scan_all_dirs(state, x, y, sign_);
                
                long long current_cell_score = 0;
                int threats_created = 0;

                for (const auto& dir_stats : directions) {
                    int s = evaluate_line(dir_stats);
                    current_cell_score += s;

                    if (s >= 1000) {
                        threats_created++;
                    }
                }

                if (threats_created >= 2) {
                    current_cell_score += 50000; 
                }

                int center_x = opts.cols / 2;
                int center_y = opts.rows / 2;
                int dist_to_center = std::abs(x - center_x) + std::abs(y - center_y);
                
                current_cell_score -= (dist_to_center * 10);

                if (current_cell_score > max_score) {
                    max_score = current_cell_score;
                    best_move = {x, y};
                }
            }
        }
    }

    return best_move;
}

}
