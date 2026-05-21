#include "player/my_player.hpp"
#include "test_stats.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    std::cout << "Testing MyPlayer vs MyPlayer\n";
    if (argc >= 2) {
        std::srand(atoi(argv[1]));
    }

    
    ttt::my_player::MyPlayer p1("MyPlayer"); ////поместите вашего игрока сюда
    ttt::my_player::MyPlayer p2("MyPlayer");
    auto result = ttt::test::run_game_tests(p1, p2, 100); //здесь вы можете изменить количество тестовых итераций ~~ 100
    
    
    ttt::test::print_test_results(result, "MyPlayer", "MyPlayer");
    
    return 0;
}