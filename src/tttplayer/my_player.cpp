#include "../player/my_player.hpp"

namespace ttt::my_player {

ttt::game::Point MyPlayer::make_move(const ttt::game::State& state) {
    const auto& opts = state.get_opts();
    
    // Просто ищем первую свободную клетку
    for (int y = 0; y < opts.rows; ++y) {
        for (int x = 0; x < opts.cols; ++x) {
            if (state.get_value(x, y) == ttt::game::Sign::NONE) {
                return ttt::game::Point{ x, y };
            }
        }
    }
    return ttt::game::Point{ 0, 0 };
}

} // namespace ttt::my_player
