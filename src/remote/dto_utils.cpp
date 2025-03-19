#include "dto_utils.hpp"

namespace ttt::remote {

using game::EventType;
using game::MoveResult;

Event translate_event(const ttt_dto::Event &ev) {
  if (ev.has_dq()) {
    auto reason = MoveResult::ERROR;
    switch (ev.dq().reason()) {
    case ttt_dto::DqReason::OUT_OF_FIELD:
      reason = MoveResult::DQ_OUT_OF_FIELD;
      break;
    case ttt_dto::DqReason::OUT_OF_ORDER:
      reason = MoveResult::DQ_OUT_OF_ORDER;
      break;
    case ttt_dto::DqReason::PLACE_OCCUPIED:
      reason = MoveResult::DQ_PLACE_OCCUPIED;
      break;
    default:
      break;
    }
    return Event::make_dq_event(translate_sign(ev.dq().sign()), reason);
  }
  if (ev.has_win()) {
    return Event::make_win_event(translate_sign(ev.win().sign()));
  }
  if (ev.has_draw()) {
    return Event::make_draw_event();
  }
  if (ev.has_move()) {
    return Event::make_move_event(ev.move().x(), ev.move().y(),
                                  translate_sign(ev.move().sign()));
  }
  if (ev.has_game_started()) {
    return Event::make_game_started_event();
  }
  if (ev.has_player_joined()) {
    return Event::make_player_joined_event(
        translate_sign(ev.player_joined().sign()),
        ev.player_joined().name().c_str());
  }
  throw "unknown error";
}

ttt_dto::Event translate_event(const Event &ev) {
  ttt_dto::Event result;
  switch (ev.type) {
  case EventType::DQ:
    switch (ev.data.dq.reason) {
    case MoveResult::DQ_PLACE_OCCUPIED:
      result.mutable_dq()->set_reason(ttt_dto::DqReason::PLACE_OCCUPIED);
      break;
    case MoveResult::DQ_OUT_OF_ORDER:
      result.mutable_dq()->set_reason(ttt_dto::DqReason::OUT_OF_ORDER);
      break;
    case MoveResult::DQ_OUT_OF_FIELD:
      result.mutable_dq()->set_reason(ttt_dto::DqReason::OUT_OF_FIELD);
      break;
    default:
      throw "unknown dq reason";
    }
    result.mutable_dq()->set_sign(translate_sign(ev.data.dq.player));
    break;
  case EventType::WIN:
    result.mutable_win()->set_sign(translate_sign(ev.data.win.player));
    break;
  case EventType::DRAW:
    (void)result.mutable_draw();
    break;
  case EventType::MOVE:
    result.mutable_move()->set_sign(translate_sign(ev.data.move.player));
    result.mutable_move()->set_x(ev.data.move.x);
    result.mutable_move()->set_y(ev.data.move.y);
    break;
  case EventType::GAME_STARTED:
    result.mutable_game_started();
    break;
  case EventType::PLAYER_JOINED:
    result.mutable_player_joined()->set_sign(
        translate_sign(ev.data.player_joined.player_sign));
    result.mutable_player_joined()->set_name(ev.data.player_joined.player_name);
    break;
  }
  return result;
}

State::Opts translate_opts(const ttt_dto::GameOptions &opts) {
  State::Opts result;
  result.cols = opts.cols();
  result.rows = opts.rows();
  result.win_len = opts.win_length();
  result.max_moves = opts.max_moves();
  return result;
}

ttt_dto::GameOptions translate_opts(const State::Opts &opts) {
  ttt_dto::GameOptions result;
  result.set_cols(opts.cols);
  result.set_rows(opts.rows);
  result.set_max_moves(opts.max_moves);
  result.set_win_length(opts.win_len);
  return result;
}

Sign translate_sign(const ttt_dto::Sign &sign) {
  switch (sign) {
  case ttt_dto::Sign::O:
    return Sign::O;
  case ttt_dto::Sign::X:
    return Sign::X;
  case ttt_dto::Sign::NONE:
    return Sign::NONE;
  default:
    return Sign::NONE;
  }
}

ttt_dto::Sign translate_sign(const Sign &sign) {
  switch (sign) {
  case Sign::X:
    return ttt_dto::Sign::X;
  case Sign::O:
    return ttt_dto::Sign::O;
  case Sign::NONE:
    return ttt_dto::Sign::NONE;
  }
}
}; // namespace ttt::remote
