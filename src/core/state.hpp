#pragma once
#include "field.hpp"

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

enum class Sign { 
  NONE,
  X,
  O,
  WALL
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
  IFieldInitializer* m_initializer;
  FieldBitmap m_field;
  int m_move_no;
  Status m_status;
  Sign m_player;
  Sign m_winner;

public:
  State(const Opts &opts, const IFieldInitializer* initializer = nullptr);
  State(const State &state);
  ~State();

  void reset();
  MoveResult process_move(Sign player, int x, int y);

  Sign get_value(int x, int y) const;
  Status get_status() const;
  Sign get_current_player() const;
  int get_move_no() const;
  const Opts &get_opts() const;
  Sign get_winner() const;

  State &operator=(const State &state) = default;

  void set_field_initializer(const IFieldInitializer* initializer);

private:
  bool _valid_coords(int x, int y) const;
  void _set_value(int x, int y, Sign sign);
  Sign _opp_sign(Sign player);
  bool _is_winning(int x, int y);
  void _reset_state();
};
}; // namespace ttt::game
