#include "state.hpp"

#include <algorithm>
#include <cstring>

namespace ttt::game {

FieldBitmap::FieldBitmap(int rows, int cols)
    : m_rows(rows), m_cols(cols), m_bitmap(0) {
  m_bitmap = new char[bitmap_size()];
  reset();
}

FieldBitmap::FieldBitmap(const FieldBitmap &other) : m_bitmap(0) {
  *this = other;
}

FieldBitmap::FieldBitmap(FieldBitmap &&other) : m_bitmap(0) {
  *this = std::move(other);
}

FieldBitmap::~FieldBitmap() { delete[] m_bitmap; }

FieldBitmap &FieldBitmap::operator=(const FieldBitmap &other) {
  if (this == &other)
    return *this;
  m_cols = other.m_cols;
  m_rows = other.m_rows;
  delete[] m_bitmap;
  m_bitmap = new char[bitmap_size()];
  std::memcpy(m_bitmap, other.m_bitmap, bitmap_size());
  return *this;
}

FieldBitmap &FieldBitmap::operator=(FieldBitmap &&other) {
  if (this == &other)
    return *this;
  m_cols = other.m_cols;
  m_rows = other.m_rows;
  delete[] m_bitmap;
  m_bitmap = other.m_bitmap;
  other.m_cols = other.m_rows = 0;
  other.m_bitmap = 0;
  return *this;
}

Sign FieldBitmap::get(int x, int y) const {
  if (!is_valid(x, y))
    return Sign::NONE;
  const int bit_no = (x + y * m_cols) * 2;
  const int byte_no = bit_no / 8;
  const char value = (m_bitmap[byte_no] >> (bit_no % 8)) & 0b11;
  switch (value) {
  case 1:
    return Sign::X;
  case 2:
    return Sign::O;
  default:
    return Sign::NONE;
  }
}

bool FieldBitmap::is_valid(int x, int y) const {
  return !(x < 0 || x >= m_cols || y < 0 || y >= m_rows);
}

void FieldBitmap::set(int x, int y, Sign s) {
  if (!is_valid(x, y))
    return;
  const int bit_no = (x + y * m_cols) * 2;
  const int offset = bit_no % 8;
  char &byte = m_bitmap[bit_no / 8];
  byte &= ~(0b11 << offset);
  if (Sign::NONE == s)
    return;
  const int value = s == Sign::X ? 1 : 2;
  byte |= value << offset;
}

void FieldBitmap::reset() { std::memset(m_bitmap, 0, bitmap_size()); }

int FieldBitmap::bitmap_size() const { return (m_rows * m_cols * 2 + 7) / 8; }

State::State(const Opts &opts) : m_opts(opts), m_field(opts.rows, opts.cols) {
  reset();
}

void State::reset() {
  const int n_cells = m_opts.rows * m_opts.cols;
  if (m_opts.max_moves == 0) {
    m_opts.max_moves = n_cells;
  }
  m_field.reset();
  m_move_no = 0;
  m_player = Sign::X;
  m_status = Status::CREATED;
  m_winner = Sign::NONE;
}

MoveResult State::process_move(Sign player, int x, int y) {
  if (m_status == Status::ENDED) {
    return MoveResult::ENDED;
  }
  if (player == Sign::NONE) {
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
      bool has_x = false, has_o = false, has_none = false;
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
        }
      }
      if (!has_none && (has_x && !has_o || has_o && !has_x)) {
        return true;
      }
    }
  }
  return false;
}

}; // namespace ttt::game
