#include "my_bot.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace ttt::my_player {

static bool initialized = false;

MyBot::MyBot(const char* name) 
    : m_name(name), m_sign(Sign::NONE) {
    if (!initialized) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        initialized = true;
    }
}

void MyBot::set_sign(Sign sign) { m_sign = sign; }
const char* MyBot::get_name() const { return m_name.c_str(); }

// зона поиска
Point MyBot::find_last_move(const State& state) const {
    int rows = state.get_opts().rows, cols = state.get_opts().cols;
    Point last = {-1, -1};
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x)
            if (state.get_value(x, y) != Sign::NONE) last = {x, y};
    return last;
}

SearchZone MyBot::get_search_zone(const State& state) const {
    int rows = state.get_opts().rows, cols = state.get_opts().cols;
    Point last = find_last_move(state);
    int cx = (last.x == -1) ? cols / 2 : last.x;
    int cy = (last.y == -1) ? rows / 2 : last.y;
    const int R = 4;
    return {
        std::max(0, cx - R), std::min(cols - 1, cx + R),
        std::max(0, cy - R), std::min(rows - 1, cy + R)
    };
}

// веса
int MyBot::get_line_weight(int length, int open_ends) const {
    if (length < 2) return 0;
    int base = (length >= 5) ? WEIGHT_WIN : (length == 4) ? WEIGHT_QUADRUPLE :
                (length == 3) ? WEIGHT_TRIPLE : WEIGHT_DOUBLE;
    if (open_ends == 0) return 0;
    return (open_ends == 1) ? base / 2 : base;
}

// сканирование
bool MyBot::is_in_bounds(const State& state, int x, int y) const {
    return x >= 0 && x < state.get_opts().cols && y >= 0 && y < state.get_opts().rows;
}

int MyBot::count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int len = 1;
    int cx = x + dx, cy = y + dy;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) { len++; cx += dx; cy += dy; }
    cx = x - dx; cy = y - dy;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) { len++; cx -= dx; cy -= dy; }
    return len;
}

int MyBot::count_open_ends(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int open = 0;
    
    // Вперёд
    int cx = x, cy = y;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) { cx += dx; cy += dy; }
    if (!is_in_bounds(state, cx, cy) || state.get_value(cx, cy) == Sign::NONE) open++;
    
    // Назад (ВНИМАНИЕ: без слова int, просто присваивание)
    cx = x; cy = y; 
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) { cx -= dx; cy -= dy; }
    if (!is_in_bounds(state, cx, cy) || state.get_value(cx, cy) == Sign::NONE) open++;
    
    return open;
}

// ход
Point MyBot::make_move(const State& state) {
    SearchZone z = get_search_zone(state);
    Point cells[400]; int cnt = 0;
    
    for (int y = z.y_start; y <= z.y_end; ++y)
        for (int x = z.x_start; x <= z.x_end; ++x)
            if (state.get_value(x, y) == Sign::NONE) cells[cnt++] = {x, y};
            
    if (cnt == 0) {
        for (int y = 0; y < state.get_opts().rows; ++y)
            for (int x = 0; x < state.get_opts().cols; ++x)
                if (state.get_value(x, y) == Sign::NONE) cells[cnt++] = {x, y};
    }
    return (cnt == 0) ? Point{0,0} : cells[std::rand() % cnt];
}

} 