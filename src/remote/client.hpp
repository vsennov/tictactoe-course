#pragma once
#include "core/game.hpp"

#include <memory>
#include <string>
#include <zmq.hpp>

namespace ttt::remote {

using game::ComposedObserver;
using game::IObserver;
using game::IPlayer;
using game::Sign;
using game::State;

class Client {
  zmq::socket_t m_sock;
  std::string m_token;
  const char *m_error = nullptr;
  ComposedObserver m_obs;
  IPlayer *m_player = nullptr;
  std::unique_ptr<State> m_state;
  int m_timelimit_ms;
  bool m_should_retry = false;

public:
  Client();
  Client(zmq::socket_t &&socket, IPlayer *player, std::string token,
         int timelimit);
  Client(const char *connection_error);
  bool is_connected() const;
  const char *get_connection_error() const;
  const std::string &get_token() const;
  void add_observer(IObserver *obs);
  bool handle_one_update(int timelimit_ms);
  void handle_all_updates();
  void close() { m_sock.close(); }
  bool should_retry() const { return m_should_retry; }

private:
  void send_ready();
  void send_move(int x, int y);
  void disconnect(const char *reason, bool should_retry);
};

class ClientContext {
  zmq::context_t m_ctx;

public:
  ClientContext();
  ~ClientContext();
  Client connect_player(const char *addr, IPlayer &player,
                        const char *password = 0);
  Client connect_observer(const char *addr, IObserver &obs,
                          const char *password = 0);
};
}; // namespace ttt::remote
