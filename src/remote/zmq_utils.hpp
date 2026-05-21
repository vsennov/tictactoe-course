#pragma once
#include <chrono>
#include <string>
#include <zmq.hpp>

namespace ttt::remote {

static bool wait_for_input(zmq::socket_t &sock, int timelimit_ms) {
  zmq::pollitem_t poller{sock, 0, ZMQ_POLLIN, 0};
  zmq::poll(&poller, 1, std::chrono::milliseconds{timelimit_ms});
  if (!(poller.revents & ZMQ_POLLIN))
    return false;
  return true;
}

template <class DtoType>
bool send_dto(zmq::socket_t &sock, const DtoType &dto) {
  std::string buf;
  if (!dto.SerializeToString(&buf))
    return false;
  zmq::message_t msg(buf);
  sock.send(msg, zmq::send_flags::none);
  return true;
}

template <class DtoType> bool recv_dto(zmq::socket_t &sock, DtoType &dto) {
  zmq::message_t msg;
  (void)sock.recv(msg);
  return dto.ParseFromString(msg.to_string());
}

}; // namespace ttt::remote
