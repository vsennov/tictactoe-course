#include "player/my_player.hpp"
#include "core/baseline.hpp"
#include "test_stats.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::cout << "Testing MyPlayer vs baseline hard player\n";
    if (argc >= 2) {
        std::srand(atoi(argv[1]));
    }


    ttt::my_player::MyPlayer p1("MyPlayer"); ////поместите вашего игрока сюда
    ttt::game::IPlayer* p2 = ttt::baseline::get_easy_player("BaselineHard"); //здесь вы можете выбрать между базовыми игроками: сложным и лёгким

    auto result = ttt::test::run_game_tests(p1, *p2, 100); //здесь вы можете изменить количество тестовых итераций ~~ 100
    //винрейт от 68-75%

    ttt::test::print_test_results(result, "MyPlayer", "BaselineHard");

    delete p2;
    return 0;
}