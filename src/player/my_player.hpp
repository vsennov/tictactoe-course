#pragma once

#include "core/game.hpp"
#include <string>
#include <vector>

namespace ttt::my_player {

using game::Event;
using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;


class MyPlayer : public game::IPlayer {
public:
    MyPlayer(const char *name);
    ~MyPlayer() override = default;
    void set_sign(Sign sign) override;
    Point make_move(const State &state) override;
    const char *get_name() const override;
    //эвристическая функция оценки всего поля
    int evaluate_board(const State &state);

private:
    Sign m_sign;
    const char *m_name;
    //вспомогательная функция для оценки одной линии из 5 клеток
    int evaluate_line(int my_count, int enemy_count);
};

} // namespace ttt::my_player


