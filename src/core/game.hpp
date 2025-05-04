#pragma once

#include "event.hpp"
#include "state.hpp"

namespace ttt::game {

class Game;

struct Point {
  int x;
  int y;
};

struct IObserver {
  virtual void handle_event(const State &game, const Event &event) {}
  virtual ~IObserver() {}
};

struct IPlayer : public IObserver {
  virtual void set_sign(Sign sign) = 0;
  virtual Point make_move(const State &) = 0;
  virtual const char *get_name() const = 0;
};

class ComposedObserver : public IObserver {
  IObserver **m_observers;

public:
  ComposedObserver();
  ComposedObserver(const ComposedObserver &obs);
  ~ComposedObserver();

  void add_observer(IObserver *observer);
  void remove_observer(IObserver *observer);
  void handle_event(const State &game, const Event &event) override;

  ComposedObserver &operator=(const ComposedObserver &obs);
};

class Game {
  ComposedObserver m_observer;
  IPlayer *m_x_player;
  IPlayer *m_o_player;
  State m_state;

public:
  Game(const State::Opts &opts);
  Game(const State &state);

  const State &get_state() const;
  const IPlayer *get_player(Sign sign) const;

  void add_player(Sign sign, IPlayer *player);
  IPlayer *remove_player(Sign sign);
  void add_observer(IObserver *observer);
  void remove_observer(IObserver *observer);

  MoveResult process();
  void reset();

  Game &operator=(const Game &game);

private:
  IPlayer *&_get_player(Sign sign);
};

}; // namespace ttt::game
