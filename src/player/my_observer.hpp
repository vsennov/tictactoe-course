#pragma once

#include "core/game.hpp"

namespace ttt::my_player {

using game::Event;
using game::IObserver;
using game::State;

class ConsoleWriter : public IObserver {
public:
  void handle_event(const State &game, const Event &event) override;
};

}; // namespace ttt::my_player
