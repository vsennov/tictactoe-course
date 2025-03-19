#include "server.hpp"
#include "client.hpp"
#include "dto_utils.hpp"
#include "zmq_utils.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <zmq_addon.hpp>

namespace ttt::remote {

using game::Game;

Point REMOTE_ERROR = {-333, -333};

bool recv_with_fallback(zmq::socket_t &sock, zmq::message_t &msg,
                        const ClientIdentity &id,
                        IFallbackServerAction &fallback, int timelimit_ms) {
  TimerMs timer;
  while (timelimit_ms >= 0) {
    if (!wait_for_input(sock, timelimit_ms)) {
      break;
    }
    timelimit_ms -= timer.get_passed_time_ms();
    auto now = std::chrono::high_resolution_clock::now();
    zmq::multipart_t resp(sock);
    if (resp.size() != 3) {
      fallback.handle_error();
      return false;
    }
    if (resp.front().to_string_view() == id) {
      msg.swap(resp[2]);
      return true;
    }
    fallback.handle_other_msg(resp.front().to_string(), resp.back());
  }
  fallback.handle_timeout();
  return true;
}

template <class DtoType>
bool recv_dto_with_fallback(zmq::socket_t &sock, DtoType &result,
                            const ClientIdentity &id,
                            IFallbackServerAction &fallback, int timelimit_ms) {
  zmq::message_t msg;
  if (!recv_with_fallback(sock, msg, id, fallback, timelimit_ms)) {
    return false;
  }
  return result.ParseFromString(msg.to_string_view());
}

template <class DtoType>
bool send_to_client(zmq::socket_t &sock, const DtoType &dto,
                    const ClientIdentity &id) {
  zmq::multipart_t msg;
  msg.addstr(id);
  msg.addstr("");
  std::string payload;
  /* std::cout << "...sending to " << id << " " << dto.DebugString() << '\n'; */
  if (!dto.SerializeToString(&payload)) {
    std::cerr << "cannot serialize client payload\n";
    return false;
  }
  msg.addstr(payload);
  msg.send(sock);
  return true;
}

RemotePlayer::RemotePlayer(zmq::socket_t &sock, const ClientIdentity &id,
                           std::string name, int timelimit_ms)
    : m_sock(sock), m_fallback(), m_id(id), m_name(name),
      m_timelimit_ms(timelimit_ms) {}

void RemotePlayer::set_fallback(IFallbackServerAction *fallback) {
  m_fallback.reset(fallback);
}

void RemotePlayer::set_sign(Sign sign) {
  ttt_dto::Update msg;
  msg.mutable_new_game()->set_sign(translate_sign(sign));
  auto opts = translate_opts(m_opts);
  msg.mutable_new_game()->set_allocated_options(new ttt_dto::GameOptions(opts));
  send_to_client(m_sock, msg, m_id);
  ttt_dto::ClientResponse resp;
  if (!recv_dto_with_fallback(m_sock, resp, m_id, *m_fallback,
                              m_timelimit_ms)) {
    return;
  }
  if (resp.type() == ttt_dto::ClientResponseType::DISCONNECT) {
    m_fallback->handle_client_disconnect();
    return;
  }
}

Point RemotePlayer::make_move(const State &) {
  ttt_dto::Update msg;
  msg.set_move_request(true);
  send_to_client(m_sock, msg, m_id);
  ttt_dto::ClientResponse resp;
  if (!recv_dto_with_fallback(m_sock, resp, m_id, *m_fallback,
                              m_timelimit_ms)) {
    return REMOTE_ERROR;
  }
  if (resp.type() == ttt_dto::ClientResponseType::DISCONNECT) {
    m_fallback->handle_client_disconnect();
    return REMOTE_ERROR;
  }
  if (!resp.has_move_x() || !resp.has_move_y()) {
    m_fallback->handle_error();
    return REMOTE_ERROR;
  }
  Point result;
  result.x = resp.move_x();
  result.y = resp.move_y();
  return result;
}

void RemotePlayer::handle_event(const State &game, const game::Event &event) {
  ttt_dto::Update msg;
  msg.set_allocated_event(new ttt_dto::Event(translate_event(event)));
  send_to_client(m_sock, msg, m_id);
  ttt_dto::ClientResponse resp;
  if (!recv_dto_with_fallback(m_sock, resp, m_id, *m_fallback,
                              m_timelimit_ms)) {
    return;
  }
  if (resp.type() == ttt_dto::ClientResponseType::DISCONNECT) {
    m_fallback->handle_client_disconnect();
    return;
  }
}

const char *RemotePlayer::get_name() const { return m_name.c_str(); }

void RemotePlayer::set_opts(const State::Opts &opts) { m_opts = opts; }

const ClientIdentity &RemotePlayer::get_id() { return m_id; }

BasicServer::BasicServer(zmq::context_t &ctx, int timelimit_ms)
    : m_sock(ctx, zmq::socket_type::router), m_timelimit_ms(timelimit_ms) {}

BasicServer::~BasicServer() { shutdown(); }

bool BasicServer::bind(const char *addr) {
  try {
    m_sock.bind(addr);
  } catch (zmq::error_t) {
    m_error = "cannot bind address";
    return false;
  }
  return true;
}

void BasicServer::set_password(const char *password) { m_password = password; }

void BasicServer::accept_players(bool accept) { m_accepting_players = accept; }

void BasicServer::accept_observers(bool accept) {
  m_accepting_observers = accept;
}

bool BasicServer::is_running() const { return m_error == 0; }

const char *BasicServer::get_error_msg() const { return m_error; }

int BasicServer::get_n_conencted_players() const { return m_players.size(); }

IPlayer *BasicServer::get_player(int i) {
  if (i < 0 || i >= get_n_conencted_players())
    return nullptr;
  auto it = m_players.begin();
  while (i-- > 0) {
    ++it;
  }
  return &(*it);
}

class RemoteObserver : public IObserver, public IFallbackServerAction {
  std::list<ClientIdentity> &m_observers;
  int m_timelimit_ms;
  zmq::socket_t &m_sock;
  ClientIdentity *m_current_id;
  BasicServer &m_server;

public:
  RemoteObserver(std::list<ClientIdentity> &observers, zmq::socket_t &sock,
                 BasicServer &server, int timelimit_ms)
      : m_observers(observers), m_sock(sock), m_timelimit_ms(timelimit_ms),
        m_server(server) {}

  void set_opts(const State::Opts &opts) {
    ttt_dto::Update new_game_msg;
    ttt_dto::ClientResponse resp;
    new_game_msg.mutable_new_game()->set_sign(ttt_dto::Sign::NONE);
    auto dto_opts = translate_opts(opts);
    new_game_msg.mutable_new_game()->mutable_options()->CopyFrom(dto_opts);
    auto observers_copy = m_observers;
    for (auto &obs_id : observers_copy) {
      m_current_id = &obs_id;
      send_to_client(m_sock, new_game_msg, obs_id);
      recv_dto_with_fallback(m_sock, resp, obs_id, *this, m_timelimit_ms);
      if (resp.type() != ttt_dto::ClientResponseType::READY)
        handle_client_disconnect();
    }
  }

  void handle_event(const State &game, const game::Event &event) override {
    if (m_observers.empty())
      return;
    ttt_dto::Update msg;
    auto event_msg = translate_event(event);
    msg.mutable_event()->CopyFrom(event_msg);
    ttt_dto::ClientResponse resp;
    auto observers_copy = m_observers;
    for (auto &obs_id : observers_copy) {
      m_current_id = &obs_id;
      send_to_client(m_sock, msg, obs_id);
      recv_dto_with_fallback(m_sock, resp, obs_id, *this, m_timelimit_ms);
      if (resp.type() != ttt_dto::ClientResponseType::READY)
        handle_client_disconnect();
    }
  }

  void handle_other_msg(const ClientIdentity &id,
                        const zmq::message_t &msg) override {
    m_server.handle_background_msg(id, msg);
  }

  void handle_timeout() override { handle_client_disconnect(); }
  void handle_error() override { handle_client_disconnect(); }
  void handle_client_disconnect() override {
    m_observers.remove(*m_current_id);
  }
};

struct RemotePlayerFallback : public IFallbackServerAction {
  BasicServer &server;
  RemotePlayer &player;

  RemotePlayerFallback(BasicServer &server_, RemotePlayer &player_)
      : server(server_), player(player_) {}

  void handle_other_msg(const ClientIdentity &id, const zmq::message_t &msg) {
    server.handle_background_msg(id, msg);
  }

  void handle_timeout() { handle_client_disconnect(); }

  void handle_error() { handle_client_disconnect(); }

  void handle_client_disconnect() { server.remove_player_by_error(player); }
};

void BasicServer::run_game(const State::Opts &opts, IPlayer *x_player,
                           IPlayer *o_player) {
  if (x_player == 0 || o_player == 0 || x_player == o_player)
    return;

  for (auto &pl : m_players)
    pl.set_opts(opts);

  Game game_instance(opts);
  game_instance.add_player(Sign::X, x_player);
  game_instance.add_player(Sign::O, o_player);
  RemoteObserver obs(m_observers, m_sock, *this, m_timelimit_ms);
  obs.set_opts(opts);
  game_instance.add_observer(&obs);
  while (game::MoveResult::OK == game_instance.process())
    ;
  game_instance.remove_observer(&obs);
}

void BasicServer::wait_for_players(int timelimit_ms) {
  TimerMs timer;
  while (timelimit_ms >= 0) {
    if (!wait_for_input(m_sock, timelimit_ms))
      break;
    timelimit_ms -= timer.get_passed_time_ms();
    if (timelimit_ms < 0)
      timelimit_ms = 0;
    zmq::multipart_t req(m_sock);
    if (req.size() != 3)
      continue;
    handle_background_msg(req.front().to_string(), req.back());
  }
}

void BasicServer::shutdown() {
  ttt_dto::Update msg;
  msg.set_server_closed(true);
  broadcast(msg);
  m_players.clear();
  m_observers.clear();
}

void BasicServer::heartbeat_players() {
  ttt_dto::Update msg;
  msg.set_ping_update(true);
  for (auto it = m_players.begin(); it != m_players.end();) {
    send_to_client(m_sock, msg, it->get_id());
    if (!receive_ready(it->get_id())) {
      it = m_players.erase(it);
    } else {
      ++it;
    }
  }
}

void BasicServer::handle_background_msg(const ClientIdentity &id,
                                        const zmq::message_t &msg) {
  drop_old_pending();
  ttt_dto::JoinRequest req;
  ttt_dto::JoinResponse resp;
  ttt_dto::ClientResponse req2;
  if (m_pending.find(id) != m_pending.end()) {
    auto it = m_pending.find(id);
    PendingClientInfo info = it->second;
    m_pending.erase(it);
    if (req2.ParseFromString(msg.to_string_view()) &&
        req2.type() == ttt_dto::ClientResponseType::READY) {
      if (info.name.empty()) {
        m_observers.push_back(id);
      } else {
        m_players.emplace_back(m_sock, id, info.name, m_timelimit_ms);
        m_players.back().set_fallback(
            new RemotePlayerFallback(*this, m_players.back()));
      }
      return;
    }
    ttt_dto::Update resp2;
    resp2.set_server_closed(true);
    send_to_client(m_sock, resp2, id);
    return;
  }
  if (req.ParseFromString(msg.to_string_view())) {
    if (!m_password.empty() &&
        (!req.has_password() || req.password() != m_password)) {
      resp.mutable_rejected()->set_reason(
          ttt_dto::JoinRejectReason::WRONG_PASSWORD);
      send_to_client(m_sock, resp, id);
      return;
    }
    if (req.join_type() == ttt_dto::MemberType::OBSERVER) {
      if (!m_accepting_observers) {
        resp.mutable_rejected()->set_reason(
            ttt_dto::JoinRejectReason::GAME_IN_PROGRESS);
      } else {
        resp.mutable_accepted()->set_timelimit_ms(m_timelimit_ms);
        m_pending[id] = {""};
      }
      send_to_client(m_sock, resp, id);
      return;
    }
    if (!m_accepting_players) {
      resp.mutable_rejected()->set_reason(
          ttt_dto::JoinRejectReason::GAME_IN_PROGRESS);
      send_to_client(m_sock, resp, id);
      return;
    }
    if (!req.has_name() || req.name().empty()) {
      resp.mutable_rejected()->set_reason(
          ttt_dto::JoinRejectReason::MALFORMED_MESSAGE);
      send_to_client(m_sock, resp, id);
      return;
    }
    if (name_taken(req.name())) {
      resp.mutable_rejected()->set_reason(
          ttt_dto::JoinRejectReason::NAME_TAKEN);
      send_to_client(m_sock, resp, id);
      return;
    }
    m_pending[id] = {req.name()};
    resp.mutable_accepted()->set_timelimit_ms(m_timelimit_ms);
    send_to_client(m_sock, resp, id);
    return;
  }
  resp.mutable_rejected()->set_reason(
      ttt_dto::JoinRejectReason::MALFORMED_MESSAGE);
}

void BasicServer::remove_player_by_error(RemotePlayer &player) {
  for (auto it = m_players.begin(); it != m_players.end(); ++it) {
    if (&(*it) == &player) {
      ttt_dto::Update msg;
      msg.set_server_closed(true);
      send_to_client(m_sock, msg, player.get_id());
      m_players.erase(it);
      return;
    }
  }
}

bool BasicServer::name_taken(const std::string &name) {
  for (auto &p : m_players)
    if (p.get_name() == name)
      return true;
  for (auto it = m_pending.begin(); it != m_pending.end(); ++it)
    if (it->second.name == name)
      return true;
  return false;
}

void BasicServer::drop_old_pending() {
  auto now = std::chrono::high_resolution_clock::now();
  for (auto it = m_pending.begin(); it != m_pending.end();) {
    if (it->second.timer.get_passed_time_ms(false) > m_timelimit_ms) {
      it = m_pending.erase(it);
    } else
      ++it;
  }
}

template <class DtoType> void BasicServer::broadcast(const DtoType &msg) {
  for (auto &p : m_players) {
    send_to_client(m_sock, msg, p.get_id());
  }
  for (auto &obs_id : m_observers) {
    send_to_client(m_sock, msg, obs_id);
  }
}

bool BasicServer::receive_ready(const ClientIdentity &id) {
  TimerMs timer;
  int timelimit_ms;
  while ((timelimit_ms = m_timelimit_ms - timer.get_passed_time_ms(false)) >=
             0 &&
         wait_for_input(m_sock, timelimit_ms)) {
    zmq::multipart_t msg(m_sock);
    if (msg.size() != 3)
      continue;
    ClientIdentity msg_id = msg[0].to_string();
    if (id != msg_id) {
      handle_background_msg(id, msg[2]);
      continue;
    }
    ttt_dto::ClientResponse resp;
    if (!resp.ParseFromString(msg[2].to_string_view()))
      return false;
    return resp.type() == ttt_dto::ClientResponseType::READY;
  }
  return false;
}

}; // namespace ttt::remote
