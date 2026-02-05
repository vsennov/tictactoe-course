#pragma once

namespace ttt::game {

enum class Sign;

class Obstacle {
  char* m_moves;
  int m_left_size;
  int m_right_size;
  int m_up_size;
  int m_down_size;
public:
  static void move_point(int& x, int& y, char move);

  Obstacle(int seq_len);
  Obstacle(const Obstacle& other) = delete;
  Obstacle& operator=(const Obstacle& other) = delete;
  int get_rsize() const {return m_right_size;}
  int get_lsize() const {return m_left_size;}
  int get_usize() const {return m_up_size;}
  int get_dsize() const {return m_down_size;}
  char get_move(int i) const {return m_moves[i];};
  int get_moves_len() const;
  ~Obstacle();
};

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
  int get_free_cells_num() const;
  int get_cols() const {return m_cols;};
  int get_rows() const {return m_rows;};

private:
  int _bitmap_size() const;

};

class IFieldInitializer {
public:
    virtual void initialize(FieldBitmap& field) = 0;
    virtual IFieldInitializer* clone() const = 0;
    virtual ~IFieldInitializer() = default;
};

class DefaultFieldInitializer: public IFieldInitializer {
public:
    void initialize(FieldBitmap& field) {};
    IFieldInitializer* clone() const {
        return new DefaultFieldInitializer();
    }
    ~DefaultFieldInitializer() = default;
};

class RandomObstaclesFI: public IFieldInitializer {
  float m_playable_part;
  int m_max_obstacle_len;
  int m_gap;
public:
  RandomObstaclesFI(float playable_part, int max_obstacle_len, int gap):
  m_max_obstacle_len(max_obstacle_len), m_playable_part(playable_part), m_gap(gap) {};
  RandomObstaclesFI(const RandomObstaclesFI& other) = default;
  
  void initialize(FieldBitmap& field);
  IFieldInitializer* clone() const {
    return new RandomObstaclesFI(*this);
  }

  ~RandomObstaclesFI() = default;
private:
  void _finalize_field(FieldBitmap& field);
  void _find_obstacle_place(const Obstacle& obstacle,
    const FieldBitmap& field, int& x, int& y, bool exhaustive);
  int _insert_obstacle(const Obstacle& obstacle,
    FieldBitmap& field, int x, int y, int max_len);
};

};