#pragma once

#include "game.hpp"

namespace ttt::baseline {

using game::IPlayer;

IPlayer *get_easy_player(const char *name);
IPlayer *get_harder_player(const char *name);

}; // namespace ttt::baseline
