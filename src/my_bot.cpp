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

// фильтрация с учётом препятствий

bool MyBot::is_valid_move(const State& state, int x, int y) const {
    // 1. Проверка границ поля
    if (!is_in_bounds(state, x, y)) {
        return false;
    }
    
    // 2. Проверка, что клетка пустая (не препятствие и не занята)
    Sign value = state.get_value(x, y);
    if (value != Sign::NONE) {
        return false;  // Клетка занята (X, O или препятствие 🧱)
    }
    
    return true;
}

int MyBot::collect_candidates(const State& state, const SearchZone& zone, Candidate* candidates) const {
    int count = 0;
    const int MAX_CANDIDATES = 400;
    
    for (int y = zone.y_start; y <= zone.y_end; ++y) {
        for (int x = zone.x_start; x <= zone.x_end; ++x) {
            if (is_valid_move(state, x, y)) {
                candidates[count].x = x;
                candidates[count].y = y;
                candidates[count].score = 0;
                count++;
                
                if (count >= MAX_CANDIDATES) {
                    return count;
                }
            }
        }
    }
    
    return count;
}

// веса
int MyBot::get_line_weight(int length, int open_ends) const {
    if (length < 2) return 0;
    int base = (length >= 5) ? WEIGHT_WIN : (length == 4) ? WEIGHT_QUADRUPLE :
                (length == 3) ? WEIGHT_TRIPLE : WEIGHT_DOUBLE;
    if (open_ends == 0) return 0;
    return (open_ends == 1) ? base / 2 : base;
}

// сканирование с учётом препятствий

bool MyBot::is_in_bounds(const State& state, int x, int y) const {
    return x >= 0 && x < state.get_opts().cols && y >= 0 && y < state.get_opts().rows;
}

int MyBot::count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int len = 1;
    
    // Идём вперёд
    int cx = x + dx, cy = y + dy;
    while (is_in_bounds(state, cx, cy)) {
        Sign cell = state.get_value(cx, cy);
        // Останавливаемся, если встретили препятствие или другой знак
        if (cell != sign) {
            break;
        }
        len++;
        cx += dx;
        cy += dy;
    }
    
    // Идём назад
    cx = x - dx;
    cy = y - dy;
    while (is_in_bounds(state, cx, cy)) {
        Sign cell = state.get_value(cx, cy);
        // Останавливаемся, если встретили препятствие или другой знак
        if (cell != sign) {
            break;
        }
        len++;
        cx -= dx;
        cy -= dy;
    }
    
    return len;
}

int MyBot::count_open_ends(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int open = 0;
    
    // Проверяем конец в направлении (dx, dy)
    int cx = x, cy = y;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        cx += dx;
        cy += dy;
    }
    
    // Конец открыт, если:
    // 1. Вышли за границы поля, ИЛИ
    // 2. Следующая клетка пустая (Sign::NONE)
    // Препятствие (Sign::OBSTACLE) или знак соперника = закрытый конец
    if (!is_in_bounds(state, cx, cy)) {
        open++;
    } else {
        Sign next_cell = state.get_value(cx, cy);
        if (next_cell == Sign::NONE) {
            open++;
        }
        // Если next_cell == Sign::OBSTACLE или другой знак → конец закрыт
    }
    
    // Проверяем конец в направлении (-dx, -dy)
    cx = x;
    cy = y;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        cx -= dx;
        cy -= dy;
    }
    
    if (!is_in_bounds(state, cx, cy)) {
        open++;
    } else {
        Sign next_cell = state.get_value(cx, cy);
        if (next_cell == Sign::NONE) {
            open++;
        }
    }
    
    return open;
}

// ход
Point MyBot::make_move(const State& state) {
    SearchZone zone = get_search_zone(state);
    
    Candidate candidates[400];
    int count = collect_candidates(state, zone, candidates);
    
    if (count == 0) {
        SearchZone full_zone = {0, state.get_opts().cols - 1, 0, state.get_opts().rows - 1};
        count = collect_candidates(state, full_zone, candidates);
    }
    
    if (count == 0) {
        return {0, 0};
    }
    
    int index = std::rand() % count;
    return {candidates[index].x, candidates[index].y};
}

} // namespace ttt::my_player