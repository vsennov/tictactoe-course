#include "my_observer.hpp"
#include "core/game.hpp"

#include <iostream>

namespace ttt::my_player {
using game::EventType;
using game::MoveResult;
using game::Sign;

static const char *print_sign(Sign sign) {
  switch (sign) {
  case Sign::X:
    return "X";
  case Sign::O:
    return "O";
  case Sign::NONE:
    return "?";
  }
  return "";
}

static const char *print_dq(MoveResult rc) {
  switch (rc) {
  case MoveResult::DQ_OUT_OF_FIELD:
    return "placing mark out of field";
  case MoveResult::DQ_PLACE_OCCUPIED:
    return "placing mark at occupied field";
  case MoveResult::DQ_OUT_OF_ORDER:
    return "playing out of order";
  default:
    return "???";
  }
}

void ConsoleWriter::handle_event(const State &state, const Event &event) {
  switch (event.type) {
  case EventType::GAME_STARTED:
    std::cout << "Game started!" << std::endl;
    return;
  case EventType::MOVE:
    std::cout << "Player " << print_sign(event.data.move.player) << " played ("
              << event.data.move.x << ", " << event.data.move.y << ")"
              << std::endl;
    //using printing function here:
    //cant call the function from observer cause need to be built into libtttcore.a ?
    //ttt::game::print_game_state(state);
    return;
  case EventType::PLAYER_JOINED:
    std::cout << "Player " << event.data.player_joined.player_name
              << " joined as "
              << print_sign(event.data.player_joined.player_sign) << std::endl;
    return;
  case EventType::DRAW:
    std::cout << "Draw!" << std::endl;
    return;
  case EventType::WIN:
    std::cout << "Player " << print_sign(event.data.win.player) << " won!"
              << std::endl;
    return;
  case EventType::DQ:
    std::cout << "Player " << print_sign(event.data.dq.player)
              << " was disqualified by " << print_dq(event.data.dq.reason)
              << std::endl;
    return;
  default:
    return;
  }
}

}; // namespace ttt::my_player
