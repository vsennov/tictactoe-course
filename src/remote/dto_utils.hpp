#include "core/game.hpp"
#include "dto.pb.h"

namespace ttt::remote {

using game::Event;
using game::Sign;
using game::State;

Event translate_event(const ttt_dto::Event &ev);
ttt_dto::Event translate_event(const Event &ev);

State::Opts translate_opts(const ttt_dto::GameOptions &opts);
ttt_dto::GameOptions translate_opts(const State::Opts &opts);

Sign translate_sign(const ttt_dto::Sign &sign);
ttt_dto::Sign translate_sign(const Sign &sign);

}; // namespace ttt::remote
