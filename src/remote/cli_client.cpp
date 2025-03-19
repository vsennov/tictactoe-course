#include "cli_utils.hpp"
#include "client.hpp"
#include "player/my_observer.hpp"
#include "player/my_player.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using ttt::my_player::ConsoleWriter;
using ttt::my_player::MyPlayer;
using ttt::remote::Client;
using ttt::remote::ClientContext;

int main(int argc, char *argv[]) {
  mycli::cli_t cli{{
      {"address", 'a', 1, "game server address", "tcp://localhost:5555"},
      {"password", 'p', 1, "game server password"},
      {"observer", 'o', 0, "connect with observer"},
      {"no-player", 'N', 0, "connect without player"},
      {"retry", 'r', 0, "retry if connection fails"},
      {"help", 'h', 0, "show this message"},
  }};
  const char *usage = "usage: cli_client [opts] {player_name}";
  auto args = cli.parse(argc - 1, argv + 1);
  if (!args.error.empty()) {
    std::cerr << "error: " << args.error << "\n";
    std::cerr << usage << '\n';
    cli.print_opts(std::cerr, 80);
    return 1;
  }
  if (args.has_flag("help")) {
    std::cout << "cli_client: simple program to connect your player "
                 "to remote game.\n"
              << usage << '\n';
    cli.print_opts(std::cout, 80);
    return 0;
  }
  const char *name = args.get_positional(0);
  if (name == nullptr) {
    std::cerr << "error: name is required, see --help\n";
    return 1;
  }
  bool retry = args.has_flag("retry");
  const char *const *kw = nullptr;
  const char *address = cli.get_default("address");
  if ((kw = args.get_keyword("address", 0)))
    address = *kw;
  const char *password = nullptr;
  if ((kw = args.get_keyword("password", 0)))
    password = *kw;
  else
    password = std::getenv("TTT_PASSWORD");
  MyPlayer p1(name);
  Client client;
  ClientContext ctx;
  ConsoleWriter obs;
  int retry_timeout = 2000;
  do {
    if (args.has_flag("no-player"))
      client = std::move(ctx.connect_observer(address, obs, password));
    else {
      client = std::move(ctx.connect_player(address, p1, password));
      if (args.has_flag("observer")) {
        client.add_observer(&obs);
      }
    }
    if (!client.is_connected()) {
      std::cerr << "connection has not succeded: "
                << client.get_connection_error() << std::endl;
      ;
      if (!retry)
        return 1;
      std::cout << "retrying in " << retry_timeout << "ms...\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(retry_timeout));
      retry_timeout *= 1.2;
    } else
      break;
  } while (retry);
  std::cout << "connected to server\n";
  if (client.get_token().empty())
    std::cout << "server has not sent any identity token\n";
  else
    std::cout << "identity token: '" << client.get_token() << "'\n";
  std::cout << "starting receiving updates...\n";
  client.handle_all_updates();
  std::cout << "client disconnected: " << client.get_connection_error() << '\n';
  client.close();
  return 0;
}
