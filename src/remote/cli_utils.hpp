#pragma once
#include <cassert>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace mycli {
struct arg_t {
  std::string lf;
  char sf;
  int arity;
  std::string help;
  std::string default_value = "";
};

struct positional_t {
  const char *value;
  int i;
};
struct flag_t {
  const arg_t *option;
};
struct keyword_t {
  const arg_t *option;
  const char *const *agrs;
};
using parsed_arg_t = std::variant<positional_t, flag_t, keyword_t>;

struct parsed_args {
  std::vector<parsed_arg_t> args;
  std::string error;

  const char *get_positional(int i) {
    for (auto &parsed : args)
      if (std::holds_alternative<positional_t>(parsed))
        if (i-- == 0)
          return std::get<positional_t>(parsed).value;
    return 0;
  }

  bool has_flag(const std::string_view &name) {
    for (auto &parsed : args)
      if (std::holds_alternative<flag_t>(parsed))
        if (std::get<flag_t>(parsed).option->lf == name)
          return true;
    return false;
  }
  const char *const *get_keyword(const std::string_view &name, int i) {
    for (auto &parsed : args)
      if (std::holds_alternative<keyword_t>(parsed)) {
        auto &kw = std::get<keyword_t>(parsed);
        if (name == kw.option->lf && i-- == 0)
          return kw.agrs;
      }
    return nullptr;
  }
};

struct cli_t {
  std::vector<arg_t> args;

  parsed_args parse(int argc, char *argv[]) {
    parsed_args result;
    std::ostringstream ss;
    int expected_args = 0, args_start = -1;
    const arg_t *opt = nullptr;
    bool ignore_opts = false;
    for (int i = 0; i < argc; ++i) {
      const char *ca = argv[i];
      if (ignore_opts) {
        positional_t token{ca, i};
        result.args.push_back(token);
        continue;
      }
      if (opt == 0) {
        if (ca[0] != '-' || ca[1] == 0) {
          positional_t token{ca, i};
          result.args.push_back(token);
          continue;
        }
        if (ca[1] == '-' && ca[2] == 0) {
          ignore_opts = true;
          continue;
        }
        if (ca[1] == '-') {
          opt = get_by_long_name(ca + 2);
          if (opt == nullptr) {
            ss << "unknown option: --" << ca + 2;
            result.error = ss.str();
            return result;
          }
          if (opt->arity == 0) {
            flag_t token{opt};
            result.args.push_back(token);
            opt = nullptr;
            continue;
          }
          expected_args = opt->arity;
          args_start = i + 1;
          continue;
        }
        for (++ca; *ca; ++ca) {
          opt = nullptr;
          for (auto &arg : args) {
            if (arg.sf == *ca) {
              opt = &arg;
              break;
            }
          }
          if (opt == nullptr) {
            ss << "unknown option: -" << *ca;
            result.error = ss.str();
            return result;
          }
          if (opt->arity == 0) {
            flag_t token{opt};
            result.args.push_back(token);
            opt = nullptr;
            continue;
          }
          if (ca[1] != 0 && opt->arity > 0) {
            ss << "expected arg after: -" << *ca << " option";
            result.error = ss.str();
            return result;
          }
          expected_args = opt->arity;
          args_start = i + 1;
        }
        continue;
      }
      if (--expected_args == 0) {
        keyword_t token{opt, argv + i};
        result.args.push_back(token);
        opt = nullptr;
      }
    }
    if (expected_args > 0) {
      ss << "expected " << opt->arity << "args after --" << opt->lf;
      result.error = ss.str();
    }
    return result;
  }

  std::ostream &print_opts(std::ostream &os, int max_width) {
    for (auto &arg : args) {
      os << "  ";
      if (arg.sf != 0)
        os << '-' << arg.sf << " /";
      else
        os << "    ";
      if (!arg.lf.empty())
        os << " --" << arg.lf;
      os << " - " << arg.help;
      if (!arg.default_value.empty())
        os << " (default: " << arg.default_value << ")";
      os << ";\n";
    }
    return os;
  }

  const char *get_default(const std::string_view &name) {
    auto opt = get_by_long_name(name);
    if (opt && !opt->default_value.empty())
      return opt->default_value.c_str();
    return 0;
  }

  arg_t *get_by_long_name(const std::string_view &name) {
    for (auto &arg : args)
      if (arg.lf == name) {
        return &arg;
      }
    return nullptr;
  }
};

struct eof_exception_t {};

inline std::string read_any_str(std::istream &s) {
  if (s.eof())
    throw eof_exception_t{};
  std::string line;
  std::getline(s, line);
  if (s.eof() && line.empty())
    throw eof_exception_t{};
  return line;
}

inline std::tuple<bool, int> read_any_int(std::istream &s) {
  if (s.eof())
    throw eof_exception_t{};
  try {
    return std::make_tuple(true, std::stoi(read_any_str(s)));
  } catch (eof_exception_t) {
    throw eof_exception_t{};
  } catch (...) {
    return std::make_tuple(false, 0);
  }
}

inline int
read_valid_int(std::istream &s, const char *prompt,
               const std::function<const char *(int)> &validator,
               const char *not_int_msg =
                   "input is not valid int. please, input valid int") {
  bool valid = false;
  int result;
  const char *error = nullptr;
  do {
    std::cout << prompt << "> ";
    std::tie(valid, result) = read_any_int(s);
    if (!valid)
      std::cout << not_int_msg << '\n';
    else if ((error = validator(result))) {
      std::cout << error << '\n';
      valid = false;
    }
  } while (!valid);
  return result;
}

struct BoundsValidator {
  BoundsValidator(int lb, int ub) : lb_(lb), ub_(ub) {
    std::ostringstream ss;
    ss << "please input value in [" << lb << "; " << ub << "]";
    error = ss.str();
  }

  const char *operator()(int value) {
    if (value < lb_ || value > ub_)
      return error.c_str();
    return 0;
  }

  int lb_;
  int ub_;
  std::string error;
};

inline int read_int_between(std::istream &s, const char *prompt, int lb,
                            int ub) {
  BoundsValidator vl(lb, ub);
  return read_valid_int(s, prompt, vl);
}

inline int select_option(std::istream &s,
                         const std::vector<std::string> &options,
                         const char *zero_option = 0) {
  int i = 0;
  for (auto &opt : options)
    std::cout << ++i << ". " << opt << '\n';
  if (zero_option)
    std::cout << "0. " << zero_option << '\n';
  const int lb = zero_option == nullptr ? 1 : 0, ub = options.size();
  return read_int_between(s, "", lb, ub);
}

inline int select_option(std::istream &s,
                         const std::initializer_list<std::string> &options,
                         const char *zero_option = 0) {
  return select_option(s, std::vector(options), zero_option);
}
}; // namespace mycli
