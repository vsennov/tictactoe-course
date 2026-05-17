#ifndef TTTPLAYER_LINE_SCAN_HPP
#define TTTPLAYER_LINE_SCAN_HPP

#include "game.hpp"
#include <array>

namespace ttt::an {

struct LineStats {
int my_cnt = 0; // кол МОИХ знаков на линии
int opp_cnt = 0; // кол знаков противника
bool front_blocked = false; // Стена в +направлении
bool back_blocked = false; // Стена в -направлении
bool front_open = false; // виден хотя бы один пустой в +направлении в пределах окна
bool back_open = false; // виден хотя бы один пустой в -направлении
int front_len = 0; // сколько клеток реально просмотрели вперёд
int back_len = 0; // сколько просмотрели назад
};

struct Dir { int dx; int dy; };

inline constexpr std::array<Dir,4> Dirs = {{
{1, 0}, // горизонталь →
{0, 1}, // вертикаль ↓
{1, 1}, // диагональ ↘
{1,-1} // диагональ ↗ (x вправо, y вверх = -1)
}};

LineStats scan_line(const ttt::game::State& st, int x, int y, Dir d, ttt::game::Sign me);

std::array<LineStats,4> scan_all_dirs(const ttt::game::State& st, int x, int y, ttt::game::Sign me);

}