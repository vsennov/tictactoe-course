#pragma once

#include "core/game.hpp"
#include <string>

namespace ttt::human_player {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

class HumanPlayer : public IPlayer {
public:
    HumanPlayer(const char* name) : m_name(name), m_sign(Sign::NONE) {}
    ~HumanPlayer() = default;

    //IPlayer interface
    void set_sign(Sign sign) override;
    Point make_move(const State& state) override;
    const char* get_name() const override;

private:
  
    //convert sign to char
    char sign_to_char(Sign sign) const;
    
    std::string m_name;
    Sign m_sign;
};

} // namespace ttt::human_player
