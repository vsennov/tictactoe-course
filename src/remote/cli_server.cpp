#include "cli_utils.hpp"
#include "server.hpp"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using ttt::game::State;
using ttt::remote::BasicServer;

int run_non_interactive(BasicServer &srv, State::Opts &opts);
int run_interactive(BasicServer &srv, State::Opts &opts);

int main(int argc, char *argv[]) {
  mycli::cli_t cli{{
      {"address", 'a', 1, "game server address", "tcp://localhost:5555"},
      {"password", 'p', 1, "game server password"},
      {"timeout", 't', 1, "client timeout in milliseconds", "300"},
      {"size", 's', 1, "length of field side", "20"},
      {"win-len", 'w', 1, "length of field side", "5"},
      {"non-interactive", 'N', 0, "run one game with two first players"},
      {"help", 'h', 0, "show this message"},
  }};
  const char *usage = "usage: cli_server [opts]";
  auto args = cli.parse(argc - 1, argv + 1);
  if (!args.error.empty()) {
    std::cerr << "error: " << args.error << "\n";
    std::cerr << usage << '\n';
    cli.print_opts(std::cerr, 80);
    return 1;
  }
  if (args.has_flag("help")) {
    std::cout << usage << '\n';
    cli.print_opts(std::cout, 80);
    return 0;
  }
  const char *const *kw = nullptr;
  const char *addr = cli.get_default("address");
  if ((kw = args.get_keyword("address", 0)))
    addr = *kw;
  const char *password = nullptr;
  if ((kw = args.get_keyword("password", 0)))
    password = *kw;
  else
    password = std::getenv("TTT_PASSWORD");
  const char *size_arg = cli.get_default("size");
  if ((kw = args.get_keyword("size", 0)))
    size_arg = *kw;
  const char *wl_arg = cli.get_default("win-len");
  if ((kw = args.get_keyword("win-len", 0)))
    wl_arg = *kw;
  const char *timeout_arg = cli.get_default("timeout");
  if ((kw = args.get_keyword("timeout", 0)))
    timeout_arg = *kw;
  int field_size = std::stoi(size_arg), win_len = std::stoi(wl_arg);
  if (field_size < 2) {
    std::cerr << "invalid field size: " << field_size
              << ". field size must be >= 2\n";
    return 1;
  }
  if (win_len < 2 || win_len > field_size) {
    std::cerr << "invalid win length: " << win_len
              << ". win length must be >= 2 and <= " << field_size << "\n";
    return 1;
  }

  State::Opts opts;
  opts.rows = opts.cols = field_size;
  opts.win_len = win_len;
  opts.max_moves = 0;

  zmq::context_t ctx;
  BasicServer server(ctx, std::stoi(timeout_arg));
  if (password)
    server.set_password(password);
  server.bind(addr);
  if (!server.is_running()) {
    std::cerr << "cannot connect to " << addr << '\n';
    return 1;
  }
  std::cout << "connected to " << addr << '\n';
  if (args.has_flag("non-interactive"))
    return run_non_interactive(server, opts);
  else
    return run_interactive(server, opts);
}

int run_non_interactive(BasicServer &srv, State::Opts &opts) {
  std::cout << "waiting for players...\n";
  int n_players = 0;
  while (srv.is_running() && srv.get_n_conencted_players() < 2) {
    srv.wait_for_players(500);
    srv.heartbeat_players();
    if (srv.get_n_conencted_players() != n_players) {
      n_players = srv.get_n_conencted_players();
      std::cout << "connected players:\n";
      for (int i = 0; i < n_players; ++i) {
        std::cout << i + 1 << ". " << srv.get_player(i)->get_name() << '\n';
      }
    }
  }
  if (!srv.is_running()) {
    std::cerr << "server error: " << srv.get_error_msg() << '\n';
    return 1;
  }
  srv.run_game(opts, srv.get_player(0), srv.get_player(1));
  return 0;
}

void bg_thread(BasicServer &srv, bool &wait, bool &exit,
               std::mutex &srv_action_mutex) {
  while (!exit) {
    while (wait) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      std::unique_lock<std::mutex> lk(srv_action_mutex);
      srv.wait_for_players(50);
    }
  }
}

void heartbeat_thread(BasicServer &srv, bool &wait, bool &exit,
                      std::mutex &srv_action_mutex) {
  while (!exit) {
    while (wait) {
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      std::unique_lock<std::mutex> lk(srv_action_mutex);
      srv.heartbeat_players();
    }
  }
}

int run_interactive(BasicServer &srv, State::Opts &opts) {
  std::cout << "waiting for players...\n";
  bool exit = false, wait = true;
  std::mutex srv_action_mutex;
  std::thread bg_t(bg_thread, std::ref(srv), std::ref(wait), std::ref(exit),
                   std::ref(srv_action_mutex));
  std::thread hb_t(heartbeat_thread, std::ref(srv), std::ref(wait),
                   std::ref(exit), std::ref(srv_action_mutex));
  while (!exit) {
    std::cout << "choose action:\n";
    try {
      switch (mycli::select_option(
          std::cin,
          {"list players", "set password", "set options", "start game"},
          "exit")) {
      case 1: {
        std::unique_lock<std::mutex> lk(srv_action_mutex);
        if (srv.get_n_conencted_players() == 0)
          std::cout << "no players yet...\n";
        else {
          for (int i = 0; i < srv.get_n_conencted_players(); ++i)
            std::cout << i + 1 << ". " << srv.get_player(i)->get_name() << '\n';
          std::cout << '\n';
        }
        break;
      }
      case 2: {
        std::cout << "enter new password: ";
        auto new_passwd = mycli::read_any_str(std::cin);
        std::unique_lock<std::mutex> lk(srv_action_mutex);
        srv.set_password(new_passwd.c_str());
        break;
      }
      case 3: {
        opts.cols = mycli::read_int_between(
            std::cin, "enter field width (2-100) ", 2, 100);
        opts.rows = mycli::read_int_between(
            std::cin, "enter field height (2-100) ", 2, 100);
        opts.win_len = mycli::read_int_between(std::cin, "enter win length ", 2,
                                               std::min(opts.cols, opts.rows));
        break;
      }
      case 4: {
        ttt::remote::IPlayer *x_player, *o_player;
        std::vector<ttt::remote::IPlayer *> players;
        std::vector<std::string> names;
        {
          std::unique_lock<std::mutex> lk(srv_action_mutex);
          if (srv.get_n_conencted_players() < 2) {
            std::cout << "not enough players\n";
            break;
          }
          for (int i = 0; i < srv.get_n_conencted_players(); ++i) {
            auto pl = srv.get_player(i);
            players.push_back(pl);
            names.push_back(pl->get_name());
          }
        }
        std::cout << "choose X player:\n";
        int x_player_no = mycli::select_option(std::cin, names, "cancel");
        if (x_player_no == 0)
          break;
        x_player = players.at(x_player_no - 1);
        players.erase(players.begin() + x_player_no - 1);
        names.erase(names.begin() + x_player_no - 1);
        int o_player_no = 1;
        if (names.size() > 1)
          o_player_no = mycli::select_option(std::cin, names, "cancel");
        if (o_player_no == 0)
          break;
        o_player = players.at(o_player_no - 1);
        {
          std::unique_lock<std::mutex> lk(srv_action_mutex);
          srv.run_game(opts, x_player, o_player);
        }
        break;
      }
      case 0: {
        exit = true;
        break;
      }
      default: {
        std::cout << "unknown action\n";
      }
      }
    } catch (mycli::eof_exception_t) {
      break;
    }
  }
  wait = false, exit = true;
  bg_t.join();
  hb_t.join();
  std::cout << "bye there...\n";
  return 0;
}
