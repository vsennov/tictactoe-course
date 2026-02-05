#include "field.hpp"
#include "state.hpp"

#include <cassert>
#include <cstring>
#include <random>

namespace ttt::game {

Obstacle::Obstacle(int seq_len) {
  m_left_size = m_right_size = m_down_size = m_up_size = 0;
  m_moves = new char[seq_len + 1];
  std::mt19937 rng(std::random_device{}());
  /*
    0 - UP (U)
    1 - LEFT (L)
    2 - DOWN (D)
    3 - RIGHT (R)
  */
  std::uniform_int_distribution<int> dist(0, 3);
  int prev_number = -1;
  int cur_x = 0;
  int cur_y = 0;
  for (int i = 0; i < seq_len; i++) {
    int cur_number = dist(rng);
    while (cur_number % 2 == prev_number % 2) { // to not make backward moves
      cur_number = dist(rng);
    }
    switch (cur_number) {
      case 0:
        m_moves[i] = 'U';
        cur_y++;
        break;
      case 1:
        m_moves[i] = 'L';
        cur_x--;
        break;
      case 2:
        m_moves[i] = 'D';
        cur_y--;
        break;
      case 3:
        m_moves[i] = 'R';
        cur_x++;
        break;
      default:
        m_moves[i] = 0;
    }
    if (cur_x <= 0 && -cur_x > m_left_size) {
      m_left_size = -cur_x;
    }
    if (cur_x >= 0 && cur_x > m_right_size) {
      m_right_size = cur_x;
    }
    if (cur_y <= 0 && -cur_y > m_down_size) {
      m_down_size = -cur_y;
    }
    if (cur_y >= 0 && cur_y > m_up_size) {
      m_up_size = cur_y;
    }
    prev_number = cur_number;
  }
  m_moves[seq_len] = 0;
}

Obstacle::~Obstacle() {
  delete[] m_moves;
}

int Obstacle::get_moves_len() const {
  return strlen(m_moves);
}

void ttt::game::Obstacle::move_point(int& x, int& y, char move) {
  switch (move) {
    case 'D':
      y--;
      break;
    case 'U':
      y++;
      break;
    case 'L':
      x--;
      break;
    case 'R':
      x++;
      break;
    default:
      break;
  }
}

int RandomObstaclesFI::_insert_obstacle(const Obstacle& obstacle,
    FieldBitmap& field, int x, int y, int max_len) {
  int i = 0;
  char move = 0;
  int cur_x = x;
  int cur_y = y;
  int inserted = 0;
  bool f_done = false;
  while (!f_done && inserted < max_len) {
    Sign cur_state = field.get(cur_x, cur_y);
    if (cur_state != Sign::WALL) {
      inserted++;
      for (int dx = -m_gap; dx <= m_gap; dx++) {
        for (int dy = -m_gap; dy <= m_gap; dy++) {
          if (dx == 0 && dy == 0) {
            field.set(cur_x, cur_y, Sign::WALL);
          }
          else if (field.get(cur_x + dx, cur_y + dy) == Sign::NONE) {
            field.set(cur_x + dx, cur_y + dy, Sign::X);
          }
        }
      }
    }
    move = obstacle.get_move(i++);
    Obstacle::move_point(cur_x, cur_y, move);
    if (!move) {
      f_done = true;
    }
  }
  return inserted;
}

void RandomObstaclesFI::_finalize_field(FieldBitmap& field) {
  for (int i = 0; i < field.get_cols(); i++) {
    for (int j = 0; j < field.get_rows(); j++) {
      Sign sign = field.get(i, j);
      if (sign != Sign::NONE && sign != Sign::WALL) {
        field.set(i, j, Sign::NONE);
      }
    }
  }
}

void RandomObstaclesFI::_find_obstacle_place(const Obstacle& obstacle,
                                                     const FieldBitmap& field,
                                                     int& x, int& y, bool exhaustive) {
  int x_start = 0;
  int y_start = 0;
  if (!exhaustive) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> x_dist(0, field.get_cols());
    std::uniform_int_distribution<int> y_dist(0, field.get_rows());
    x_start = x_dist(rng);
    y_start = y_dist(rng);  
  }
  for (int i = y_start; i < field.get_rows(); i++) {
    for (int j = x_start; j < field.get_cols(); j++) {
      int x_to_check = j;
      int y_to_check = i;
      bool f_fit = true;
      int move_i = 0;
      char move = 1;
      while (move) {
        if (field.get(x_to_check, y_to_check) != Sign::NONE) {
          f_fit = false;
          break;
        }
        move = obstacle.get_move(move_i++); 
        Obstacle::move_point(x_to_check, y_to_check, move);
      }
      if (f_fit) {
        x = j;
        y = i;
        return;
      }
    }
  }
  x = -1;
  y = -1;
}

void RandomObstaclesFI::initialize(FieldBitmap& field) {
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(0, m_max_obstacle_len);
  const int max_tries = int(sqrt(field.get_cols() * field.get_rows()));
  int place_to_fill = (int)round(field.get_cols() * field.get_rows() * (1.f - m_playable_part));
  bool exhaustive = false;
  while (place_to_fill > 0) {
    int tries_n = 0;
    while (tries_n < max_tries) {
      int cur_seq_len = dist(rng);
      Obstacle obstacle(cur_seq_len);
      int x = -1;
      int y = -1;
      _find_obstacle_place(obstacle, field, x, y, exhaustive);
      if (x >= 0 && y >= 0) {
        int inserted_n = _insert_obstacle(obstacle, field, x, y, place_to_fill);
        place_to_fill -= inserted_n;
        break;
      }
      tries_n++;
    }
    if (tries_n == max_tries) {
      if (exhaustive) {
        _finalize_field(field);
        return;
      }
      exhaustive = true;
    }
  }
  _finalize_field(field);
}

void FieldBitmap::reset() {
  std::memset(m_bitmap, 0, _bitmap_size());
}

int FieldBitmap::get_free_cells_num() const {
  int free_cells_n = 0;
  for (int i = 0; i < m_cols; i++) {
    for (int j = 0; j < m_rows; j++) {
      if (get(i, j) == Sign::NONE) {
        free_cells_n++;
      }
    }
  }
  return free_cells_n;
}

int FieldBitmap::_bitmap_size() const { return (m_rows * m_cols * 2 + 7) / 8; }

FieldBitmap::FieldBitmap(int rows, int cols)
    : m_rows(rows), m_cols(cols), m_bitmap(nullptr) {
  m_bitmap = new char[_bitmap_size()];
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
  m_bitmap = new char[_bitmap_size()];
  std::memcpy(m_bitmap, other.m_bitmap, _bitmap_size());
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
    return Sign::WALL;
  const int bit_no = (x + y * m_cols) * 2;
  const int byte_no = bit_no / 8;
  const char value = (m_bitmap[byte_no] >> (bit_no % 8)) & 0b11;
  return static_cast<Sign>(value);
}

bool FieldBitmap::is_valid(int x, int y) const {
  return !(x < 0 || x >= m_cols || y < 0 || y >= m_rows);
}

void FieldBitmap::set(int x, int y, Sign s) {
  const int bit_no = (x + y * m_cols) * 2;
  const int offset = bit_no % 8;
  char &byte = m_bitmap[bit_no / 8];
  byte &= ~(0b11 << offset);
  const int value = static_cast<int>(s);
  assert (value >= 0 && value < 4);
  byte |= value << offset;
}


};  // namespace ttt::game
