#include <iostream>
#include "game.hpp"
#include "tttplayer/my_player.hpp"

using namespace ttt::game;

int main() {
    State::Opts opts{20, 20, 5, 0};
    RandomObstaclesFI obstacles(0.75f, 50, 1);
    Game game(opts, &obstacles);

    ttt::my_player::MyPlayer px("px");
    ttt::my_player::MyPlayer po("po");
    game.add_player(Sign::X, &px);
    game.add_player(Sign::O, &po);

    while (true) {
        MoveResult r = game.process();
        if (r == MoveResult::ENDED) break;
        if (is_dq(r) || r == MoveResult::ERROR) return 1;
    }
    std::cout << "Smoke test passed!" << std::endl;
    return 0;
}

