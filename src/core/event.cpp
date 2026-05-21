#include "event.hpp"

namespace ttt::game {

Event Event::make_draw_event() {
  Event result;
  result.type = EventType::DRAW;
  return result;
}

Event Event::make_win_event(Sign player) {
  Event result;
  result.type = EventType::WIN;
  result.data.win.player = player;
  return result;
}

Event Event::make_move_event(int x, int y, Sign player) {
  Event result;
  result.type = EventType::MOVE;
  result.data.move.player = player;
  result.data.move.x = x;
  result.data.move.y = y;
  return result;
}

Event Event::make_dq_event(Sign player, MoveResult reason) {
  Event result;
  result.type = EventType::DQ;
  result.data.dq.player = player;
  result.data.dq.reason = reason;
  return result;
}

Event Event::make_player_joined_event(Sign player, const char *player_name) {
  Event result;
  result.type = EventType::PLAYER_JOINED;
  result.data.player_joined.player_name = player_name;
  result.data.player_joined.player_sign = player;
  return result;
}

Event Event::make_game_started_event() {
  Event result;
  result.type = EventType::GAME_STARTED;
  return result;
}

}; // namespace ttt::game
