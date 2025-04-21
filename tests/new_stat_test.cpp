#include "player/my_player.hpp"
#include "core/baseline.hpp"
#include "test_stats.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    std::cout << "Testing MyPlayer vs baseline easy player\n";
    if (argc >= 2) {
        std::srand(atoi(argv[1]));
    }

    // Create players
    ttt::my_player::MyPlayer p1("MyPlayer");
    ttt::game::IPlayer *p2 = ttt::baseline::get_easy_player("BaselineEasy");

    // Run tests
    auto result = ttt::test::run_game_tests(p1, *p2, 100); // 100 iterations
    
    // Print results
    ttt::test::print_test_results(result, "MyPlayer", "BaselineEasy");
    
    delete p2;
    return 0;
}