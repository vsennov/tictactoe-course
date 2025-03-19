#include "client.hpp"

#include "dto_utils.hpp"
#include "zmq_utils.hpp"
#include <algorithm>
#include <cassert>

namespace ttt::remote {

using game::Event;
using game::EventType;

Client::Client() : Client("not connected yet") {}

Client::Client(zmq::socket_t &&socket, IPlayer *player, std::string token,
               int timelimit)
    : m_sock(std::move(socket)), m_player(player), m_token(std::move(token)),
      m_timelimit_ms(timelimit) {
  send_ready();
  if (player)
    m_obs.add_observer(player);
}

Client::Client(const char *connection_error) : m_error(connection_error) {}

bool Client::is_connected() const { return m_error == nullptr; }

const char *Client::get_connection_error() const { return m_error; }

const std::string &Client::get_token() const { return m_token; }

void Client::add_observer(IObserver *obs) { m_obs.add_observer(obs); }

bool Client::handle_one_update(int timelimit_ms) {
  if (!is_connected())
    return true;
  if (!wait_for_input(m_sock, timelimit_ms))
    return false;
  ttt_dto::Update update;
  if (!recv_dto(m_sock, update)) {
    std::cerr << "bad message, disconnecting...\n";
    disconnect("bad message");
    return true;
  }
  if (update.has_server_closed()) {
    std::cerr << "server closed connection\n";
    m_error = "";
    return true;
  }
  if (update.has_ping_update()) {
    send_ready();
    return true;
  }
  if (update.has_new_game()) {
    if (!update.new_game().has_options()) {
      std::cerr << "server sent new game message without options...\n";
      disconnect("bad message");
      return true;
    }
    if (m_player) {
      const auto sign = translate_sign(update.new_game().sign());
      if (sign == Sign::NONE) {
        std::cerr << "server sent sign NONE for player...\n";
        disconnect("bad message");
        return true;
      }
      m_player->set_sign(translate_sign(update.new_game().sign()));
    }
    const auto opts = translate_opts(update.new_game().options());
    m_state.reset(new State(opts));
    send_ready();
    return true;
  }
  if (!m_state && (update.has_event() || update.has_move_request())) {
    std::cerr << "server sent request without starting game...\n";
    disconnect("unexpected message from server");
    return true;
  }
  if (update.has_event()) {
    auto event = translate_event(update.event());
    if (event.type == EventType::MOVE) {
      m_state->process_move(event.data.move.player, event.data.move.x,
                            event.data.move.y);
    }
    m_obs.handle_event(*m_state, event);
    send_ready();
    return true;
  }
  if (update.has_move_request()) {
    if (m_player == 0) {
      std::cerr << "server sent move request for observer\n";
      disconnect("bad server request");
      return true;
    }
    auto pt = m_player->make_move(*m_state);
    send_move(pt.x, pt.y);
    return true;
  }
  std::cerr << "server sent malformed update, disconnecting...\n";
  disconnect("malformed update");
  return true;
}

void Client::handle_all_updates() {
  while (is_connected()) {
    handle_one_update(1000);
  }
}

void Client::send_ready() {
  ttt_dto::ClientResponse resp;
  resp.set_type(ttt_dto::ClientResponseType::READY);
  send_dto(m_sock, resp);
}

void Client::send_move(int x, int y) {
  ttt_dto::ClientResponse resp;
  resp.set_type(ttt_dto::ClientResponseType::READY);
  resp.set_move_x(x);
  resp.set_move_y(y);
  send_dto(m_sock, resp);
}

void Client::disconnect(const char *reason) {
  ttt_dto::ClientResponse resp;
  m_error = reason;
  resp.set_type(ttt_dto::ClientResponseType::DISCONNECT);
  send_dto(m_sock, resp);
}

ClientContext::ClientContext() { GOOGLE_PROTOBUF_VERIFY_VERSION; }

ClientContext::~ClientContext() {
  google::protobuf::ShutdownProtobufLibrary();
  m_ctx.shutdown();
}

static Client connect(zmq::context_t &ctx, const char *addr, IPlayer *player,
                      ttt_dto::JoinRequest &join_req) {
  zmq::socket_t sock(ctx, zmq::socket_type::req);
  sock.set(zmq::sockopt::linger, 0);
  try {
    sock.connect(addr);
  } catch (zmq::error_t) {
    return Client("cannot connect to addr");
  }
  std::cerr << "sending join request to " << addr << "...\n";
  send_dto(sock, join_req);
  if (!wait_for_input(sock, 1500)) {
    return Client("server response timeout");
  }
  ttt_dto::JoinResponse resp;
  if (!recv_dto(sock, resp)) {
    return Client("malformed join response from server");
  }
  if (resp.has_rejected()) {
    switch (resp.rejected().reason()) {
    case ttt_dto::JoinRejectReason::WRONG_PASSWORD:
      return Client("rejected: wrong password");
    case ttt_dto::JoinRejectReason::GAME_IN_PROGRESS:
      return Client("rejected: game in progress");
    case ttt_dto::JoinRejectReason::MALFORMED_MESSAGE:
      return Client("rejected: server cannot decode out message");
    case ttt_dto::JoinRejectReason::NAME_TAKEN:
      return Client("rejected: this name was already taken");
    default:
      return Client("rejected: unknown reason");
    }
  }
  if (!resp.has_accepted()) {
    std::cerr << "server sent bad join response...\n";
    return Client("bad join response");
  }
  int timelimit_ms = -1;
  if (resp.accepted().has_timelimit_ms()) {
    timelimit_ms = resp.accepted().timelimit_ms();
  }
  std::string token;
  if (resp.accepted().has_token()) {
    token = resp.accepted().token();
  }
  return Client(std::move(sock), player, token, timelimit_ms);
}

Client ClientContext::connect_player(const char *addr, IPlayer &player,
                                     const char *password) {
  ttt_dto::JoinRequest req;
  req.set_name(player.get_name());
  if (password) {
    req.set_password(password);
  }
  req.set_join_type(ttt_dto::MemberType::PLAYER);
  return connect(m_ctx, addr, &player, req);
}

Client ClientContext::connect_observer(const char *addr, IObserver &obs,
                                       const char *password) {
  ttt_dto::JoinRequest req;
  if (password) {
    req.set_password(password);
  }
  req.set_join_type(ttt_dto::MemberType::OBSERVER);
  auto client = connect(m_ctx, addr, 0, req);
  client.add_observer(&obs);
  return client;
}
}; // namespace ttt::remote
