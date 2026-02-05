#include "state.hpp"

#include <algorithm>
#include <cstring>

namespace ttt::game {

void State::_reset_state() {
  const int max_possible_moves = m_field.get_free_cells_num();
  if (m_opts.max_moves == 0 || m_opts.max_moves > max_possible_moves) {
    m_opts.max_moves = max_possible_moves;
  }
  m_move_no = 0;
  m_player = Sign::X;
  m_status = Status::CREATED;
  m_winner = Sign::NONE;
}

State::State(const Opts &opts, const IFieldInitializer* initializer): 
  m_opts(opts), m_field(opts.rows, opts.cols) {
  if (initializer) {
    m_initializer = initializer->clone();
  }
  else {
    m_initializer = new DefaultFieldInitializer();
  }
  reset();
}

State::State(const State& state): m_opts(state.m_opts), m_field(state.m_field),
  m_player(state.m_player), m_status(state.m_status), m_winner(state.m_winner),
  m_move_no(state.m_move_no) {
  m_initializer = state.m_initializer->clone();
}

State::~State() {
  delete m_initializer;
}

void State::reset() {
  m_field.reset();
  m_initializer->initialize(m_field);
  _reset_state();
}

MoveResult State::process_move(Sign player, int x, int y) {
  if (m_status == Status::ENDED) {
    return MoveResult::ENDED;
  }
  if (player == Sign::NONE || player == Sign::WALL) {
    return MoveResult::ERROR;
  }
  if (player != m_player) {
    return MoveResult::DQ_OUT_OF_ORDER;
  }
  if (!_valid_coords(x, y)) {
    return MoveResult::DQ_OUT_OF_FIELD;
  }
  if (get_value(x, y) != Sign::NONE) {
    return MoveResult::DQ_PLACE_OCCUPIED;
  }
  _set_value(x, y, player);
  ++m_move_no;
  m_player = _opp_sign(player);
  const bool winning = _is_winning(x, y);
  if (m_status == Status::LAST_MOVE) {
    m_status = Status::ENDED;
    if (winning) {
      return MoveResult::DRAW;
    } else {
      m_winner = m_player;
      return MoveResult::WIN;
    }
  }
  m_status = Status::ACTIVE;
  if (winning) {
    if (m_move_no % 2 == 0) {
      m_status = Status::ENDED;
      m_winner = _opp_sign(m_player);
      return MoveResult::WIN;
    } else if (m_move_no >= m_opts.max_moves) {
      m_winner = _opp_sign(m_player);
      m_status = Status::ENDED;
      return MoveResult::WIN;
    } else {
      m_status = Status::LAST_MOVE;
      return MoveResult::OK;
    }
  }
  if (m_move_no >= m_opts.max_moves) {
    m_status = Status::ENDED;
    return MoveResult::DRAW;
  }
  return MoveResult::OK;
}

Sign State::get_value(int x, int y) const { return m_field.get(x, y); }

Status State::get_status() const { return m_status; }

Sign State::get_current_player() const { return m_player; }

int State::get_move_no() const { return m_move_no; }

const State::Opts &State::get_opts() const { return m_opts; }

Sign State::get_winner() const { return m_winner; }

bool State::_valid_coords(int x, int y) const { return m_field.is_valid(x, y); }

void State::_set_value(int x, int y, Sign sign) { m_field.set(x, y, sign); }

Sign State::_opp_sign(Sign player) {
  switch (player) {
  case Sign::X:
    return Sign::O;
  case Sign::O:
    return Sign::X;
  default:
    return Sign::NONE;
  }
}

bool State::_is_winning(int x, int y) {
  static const struct {
    int dx;
    int dy;
  } directions[] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
  for (const auto dir : directions) {
    for (int n = 0; n < m_opts.win_len; ++n) {
      bool has_x = false, has_o = false, has_none = false, has_wall = false;
      for (int i = 0; i < m_opts.win_len; ++i) {
        const int dn = n - i;
        switch (get_value(x + dir.dx * dn, y + dir.dy * dn)) {
        case Sign::X:
          has_x = true;
          break;
        case Sign::O:
          has_o = true;
          break;
        case Sign::NONE:
          has_none = true;
          break;
        case Sign::WALL:
          has_wall = true;
          break;
        default:
          // There was an error message here. It's gone now.
          break;
        }
      }
      if (!has_none && !has_wall && (has_x && !has_o || has_o && !has_x)) {
        return true;
      }
    }
  }
  return false;
}

void State::set_field_initializer(const IFieldInitializer* initializer) {
  delete m_initializer;
  if (initializer) {
    m_initializer = initializer->clone();
  }
  else {
    m_initializer = new DefaultFieldInitializer();
  }
}

};  // namespace ttt::game
