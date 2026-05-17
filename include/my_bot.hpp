#pragma once

#include "core/game.hpp"
#include <string>

namespace ttt::my_player {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

/**
 * @brief простой бот, делающий случайные легальные ходы
 */
class MyBot : public IPlayer {
public:
    /**
     * @brief
     * @param name
     */
    explicit MyBot(const char* name = "MyBot");
    
    /**
     * @brief
     */
    ~MyBot() override = default;

    void set_sign(Sign sign) override;
    Point make_move(const State& state) override;
    const char* get_name() const override;

private:
    std::string m_name;
    Sign m_sign;
};

}