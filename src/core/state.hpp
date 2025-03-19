#pragma once

namespace ttt::game {

enum class Status { CREATED, ACTIVE, LAST_MOVE, ENDED };

enum class MoveResult {
  OK,
  DRAW,
  WIN,
  ENDED,
  DQ_OUT_OF_FIELD,
  DQ_OUT_OF_ORDER,
  DQ_PLACE_OCCUPIED,
  ERROR,
};

inline bool is_dq(MoveResult r) {
  return r == MoveResult::DQ_OUT_OF_FIELD || r == MoveResult::DQ_OUT_OF_ORDER ||
         r == MoveResult::DQ_PLACE_OCCUPIED;
}

enum class Sign { X, O, NONE };

class FieldBitmap {
  char *m_bitmap;
  int m_rows;
  int m_cols;

public:
  FieldBitmap(int rows, int cols);
  FieldBitmap(const FieldBitmap &other);
  FieldBitmap(FieldBitmap &&other);
  ~FieldBitmap();

  FieldBitmap &operator=(const FieldBitmap &other);
  FieldBitmap &operator=(FieldBitmap &&other);

  void set(int x, int y, Sign s);
  void reset();

  Sign get(int x, int y) const;
  bool is_valid(int x, int y) const;

private:
  int bitmap_size() const;
};

class State {
public:
  struct Opts {
    int rows;
    int cols;
    int win_len;
    int max_moves;
  };

private:
  Opts m_opts;

  FieldBitmap m_field;
  int m_move_no;
  Status m_status;
  Sign m_player;
  Sign m_winner;

public:
  State(const Opts &opts);
  State(const State &state) = default;
  ~State() = default;

  void reset();
  MoveResult process_move(Sign player, int x, int y);

  Sign get_value(int x, int y) const;
  Status get_status() const;
  Sign get_current_player() const;
  int get_move_no() const;
  const Opts &get_opts() const;
  Sign get_winner() const;

  State &operator=(const State &state) = default;

private:
  bool _valid_coords(int x, int y) const;
  void _set_value(int x, int y, Sign sign);
  Sign _opp_sign(Sign player);
  bool _is_winning(int x, int y);
};
}; // namespace ttt::game
