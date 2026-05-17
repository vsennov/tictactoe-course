#include <chrono>
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

    auto t0 = std::chrono::high_resolution_clock::now();
    game.process();
    auto t1 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "Move time: " << duration << " ms" << std::endl;

    return 0;
}

