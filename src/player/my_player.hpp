#pragma once

#include "core/game.hpp"

namespace ttt::my_player {

using game::Event;
using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

struct MoveStats {
        int wins = 0;
        int open_fours = 0;
        int fours = 0;
        int open_threes = 0;
        int threes = 0;
        int twos = 0;
    };

class MyPlayer : public IPlayer {
  Sign m_sign = Sign::NONE;
  const char *m_name;

public:
  MyPlayer(const char *name) : m_sign(Sign::NONE), m_name(name) {}
  void set_sign(Sign sign) override;
  Point make_move(const State &game) override;
  const char *get_name() const override;

private:
  MoveStats evaluate_move(const State& state, int cx, int cy, Sign color) const;
};

}; // namespace ttt::my_player
