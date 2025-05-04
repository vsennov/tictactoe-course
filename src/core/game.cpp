#include "game.hpp"

namespace ttt::game {

Game::Game(const State::Opts &opts) : Game(State(opts)) {}

Game::Game(const State &state)
    : m_observer(), m_x_player(0), m_o_player(0), m_state(state) {}

const State &Game::get_state() const { return m_state; }

const IPlayer *Game::get_player(Sign sign) const {
  switch (sign) {
  case Sign::X:
    return m_x_player;
  case Sign::O:
    return m_o_player;
  default:
    return 0;
  }
}

void Game::add_player(Sign sign, IPlayer *player) {
  IPlayer *&pp = _get_player(sign);
  if (pp == player)
    return;
  remove_observer(pp);
  add_observer(player);
  pp = player;
}

IPlayer *Game::remove_player(Sign sign) {
  IPlayer *&pp = _get_player(sign);
  IPlayer *result = pp;
  remove_observer(result);
  pp = 0;
  return result;
}

void Game::add_observer(IObserver *obs) { m_observer.add_observer(obs); }
void Game::remove_observer(IObserver *obs) { m_observer.remove_observer(obs); }

MoveResult Game::process() {
  if (m_state.get_status() == Status::ENDED) {
    return MoveResult::ENDED;
  }
  if (m_state.get_status() == Status::CREATED) {
    if (!m_x_player || !m_o_player) {
      return MoveResult::ERROR;
    }
    m_x_player->set_sign(Sign::X);
    m_o_player->set_sign(Sign::O);
    m_observer.handle_event(m_state, Event::make_player_joined_event(
                                         Sign::X, m_x_player->get_name()));
    m_observer.handle_event(m_state, Event::make_player_joined_event(
                                         Sign::O, m_o_player->get_name()));
    m_observer.handle_event(m_state, Event::make_game_started_event());
  }
  Sign sign = m_state.get_current_player();
  IPlayer *p = _get_player(sign);
  if (p == 0) {
    return MoveResult::ERROR;
  }
  Point pt = p->make_move(m_state);
  MoveResult result = m_state.process_move(sign, pt.x, pt.y);
  m_observer.handle_event(m_state, Event::make_move_event(pt.x, pt.y, sign));
  switch (result) {
  case MoveResult::WIN:
    m_observer.handle_event(m_state,
                            Event::make_win_event(m_state.get_winner()));
    break;
  case MoveResult::DRAW:
    m_observer.handle_event(m_state, Event::make_draw_event());
    break;
  case MoveResult::DQ_OUT_OF_ORDER:
  case MoveResult::DQ_PLACE_OCCUPIED:
  case MoveResult::DQ_OUT_OF_FIELD:
    m_observer.handle_event(m_state, Event::make_dq_event(sign, result));
    break;
  default:
    break;
  }
  return result;
}

void Game::reset() { m_state.reset(); }

IPlayer *&Game::_get_player(Sign sign) {
  switch (sign) {
  case Sign::O:
    return m_o_player;
  case Sign::X:
    return m_x_player;
  default:
    throw "error";
  }
}

ComposedObserver::ComposedObserver() : m_observers(new IObserver *[1] { 0 }) {}
ComposedObserver::ComposedObserver(const ComposedObserver &obs)
    : m_observers(0) {
  *this = obs;
}

ComposedObserver::~ComposedObserver() { delete[] m_observers; }

void ComposedObserver::add_observer(IObserver *obs) {
  if (!obs)
    return;
  IObserver **pt;
  for (pt = m_observers; *pt; ++pt)
    if (*pt == obs)
      return;
  const int n = pt - m_observers + 1;
  pt = new IObserver *[n + 1] {};
  for (int i = 0; i < n - 1; ++i) {
    pt[i] = m_observers[i];
  }
  pt[n - 1] = obs;
  pt[n] = 0;
  delete[] m_observers;
  m_observers = pt;
}

void ComposedObserver::remove_observer(IObserver *obs) {
  if (!obs)
    return;
  for (IObserver **pt1 = m_observers, **pt2 = m_observers; *pt2; ++pt1, ++pt2) {
    if (*pt1 == obs) {
      ++pt2;
    }
    *pt1 = *pt2;
  }
}

void ComposedObserver::handle_event(const State &state, const Event &event) {
  for (IObserver **pt = m_observers; *pt; ++pt) {
    (*pt)->handle_event(state, event);
  }
}

ComposedObserver &ComposedObserver::operator=(const ComposedObserver &obs) {
  if (this == &obs)
    return *this;
  int n = 0;
  for (; obs.m_observers[n]; ++n)
    ;
  delete[] m_observers;
  m_observers = new IObserver *[n];
  for (n = 0; obs.m_observers[n]; ++n)
    m_observers[n] = obs.m_observers[n];
  m_observers[n] = 0;
  return *this;
}

}; // namespace ttt::game
