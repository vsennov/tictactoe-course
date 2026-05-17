#include <iostream>
#include "game.hpp"
#include "tttplayer/my_player.hpp"
using namespace ttt::game;

int main() {
State::Opts opts{5, 5, 5, 0};
RandomObstaclesFI obstacles(1.0f, 0, 0);
Game game(opts, &obstacles);
ttt::my_player::MyPlayer px("px");
ttt::my_player::MyPlayer po("po");
game.add_player(Sign::X, &px);
game.add_player(Sign::O, &po);

MoveResult r = game.process();
if (is_dq(r) || r == MoveResult::ERROR) {
    std::cerr << "DQ or ERROR\n";
    return 1;
}

bool found = false;
int fx=-1, fy=-1;
for (int y = 0; y < 5; ++y) {
    for (int x = 0; x < 5; ++x) {
        if (game.get_state().get_value(x,y) == Sign::X) {
            found = true; fx = x; fy = y;
        }
    }
}
if (!found) {
    std::cerr << "No move found\n";
    return 2;
}
std::cout << "Bot moved to: (" << fx << ", " << fy << ")\n";
return 0;
}