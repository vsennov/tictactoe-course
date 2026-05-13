#pragma once

#include "core/game.hpp"
#include <string>

namespace ttt::my_player {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

class MyBot : public IPlayer {
public:
    explicit MyBot(const char* name = "MyBot");

    ~MyBot() override = default;

    void set_sign(Sign sign) override;
    Point make_move(const State& state) override;
    const char* get_name() const override;

private:
    std::string m_name; 
    Sign m_sign;

};

}