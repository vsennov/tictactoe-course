#pragma once

#include "state.hpp"

namespace ttt::game {

enum class EventType {
  GAME_STARTED,
  PLAYER_JOINED,
  MOVE,
  WIN,
  DRAW,
  DQ,
};

struct Event {
  EventType type;
  union {
    struct {
      Sign player_sign;
      const char *player_name;
    } player_joined;

    struct {
      int x;
      int y;
      Sign player;
    } move;

    struct {
      Sign player;
    } win;

    struct {
      Sign player;
      MoveResult reason;
    } dq;

  } data;

  static Event make_game_started_event();
  static Event make_player_joined_event(Sign player, const char *player_name);
  static Event make_move_event(int x, int y, Sign player);
  static Event make_win_event(Sign player);
  static Event make_dq_event(Sign player, MoveResult reason);
  static Event make_draw_event();
};
}; // namespace ttt::game
