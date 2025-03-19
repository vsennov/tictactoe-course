#pragma once
#include "core/game.hpp"
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <zmq.hpp>

namespace ttt::remote {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

using ClientIdentity = std::string;

struct IFallbackServerAction {
  virtual ~IFallbackServerAction() = default;
  virtual void handle_other_msg(const ClientIdentity &id,
                                const zmq::message_t &msg) = 0;
  virtual void handle_timeout() = 0;
  virtual void handle_error() = 0;
  virtual void handle_client_disconnect() = 0;
};

class RemotePlayer : public IPlayer {
  zmq::socket_t &m_sock;
  std::unique_ptr<IFallbackServerAction> m_fallback;
  State::Opts m_opts;
  std::string m_name;
  ClientIdentity m_id;
  int m_timelimit_ms;

public:
  RemotePlayer(zmq::socket_t &sock, const ClientIdentity &id, std::string name,
               int timelimit_ms);
  void set_fallback(IFallbackServerAction *fallback);

  void set_sign(Sign sign) override;
  Point make_move(const State &) override;
  const char *get_name() const override;
  void handle_event(const State &game, const game::Event &event) override;

  void set_opts(const State::Opts &opts);
  const ClientIdentity &get_id();
};

struct TimerMs {
  std::chrono::time_point<std::chrono::high_resolution_clock> now =
      std::chrono::high_resolution_clock::now();

  int get_passed_time_ms(bool update = true) {
    auto new_now = std::chrono::high_resolution_clock::now();
    int result =
        std::chrono::duration_cast<std::chrono::milliseconds>(new_now - now)
            .count();
    if (update)
      now = new_now;
    return result;
  }
};

class BasicServer {
  friend class RemotePlayerFallback;
  friend class RemoteObserver;

  struct PendingClientInfo {
    std::string name;
    TimerMs timer;
  };

  zmq::socket_t m_sock;
  std::string m_password;
  int m_timelimit_ms;
  bool m_accepting_players = true;
  bool m_accepting_observers = true;
  const char *m_error = 0;
  std::list<ClientIdentity> m_observers;
  std::list<RemotePlayer> m_players;
  std::unordered_map<ClientIdentity, PendingClientInfo> m_pending;

public:
  BasicServer(zmq::context_t &ctx, int timelimit_ms);
  ~BasicServer();

  bool bind(const char *addr);
  void set_password(const char *password);
  void accept_players(bool accept);
  void accept_observers(bool accept);

  bool is_running() const;
  const char *get_error_msg() const;

  int get_n_conencted_players() const;
  IPlayer *get_player(int i);

  void run_game(const State::Opts &opts, IPlayer *x_player, IPlayer *o_player);
  void wait_for_players(int timelimit_ms);
  void shutdown();
  void heartbeat_players();

private:
  void handle_background_msg(const ClientIdentity &id,
                             const zmq::message_t &msg);
  void remove_player_by_error(RemotePlayer &player);
  bool name_taken(const std::string &name);
  void drop_old_pending();
  bool receive_ready(const ClientIdentity &id);

  template <class DtoType> void broadcast(const DtoType &msg);
};

}; // namespace ttt::remote
