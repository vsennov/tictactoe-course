#include "my_bot.hpp"
#include <cstdlib>
#include <ctime>

namespace ttt::my_player {

static bool initialized = false;

MyBot::MyBot(const char* name) 
    : m_name(name), m_sign(Sign::NONE) {
    if (!initialized) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        initialized = true;
    }
}

void MyBot::set_sign(Sign sign) {
    m_sign = sign;
}

const char* MyBot::get_name() const {
    return m_name.c_str();
}

int MyBot::get_line_weight(int length, int open_ends) const {
    // Базовый вес в зависимости от длины
    int base_weight = 0;
    
    if (length >= 5) {
        base_weight = WEIGHT_WIN;
    } else if (length == 4) {
        base_weight = WEIGHT_QUADRUPLE;
    } else if (length == 3) {
        base_weight = WEIGHT_TRIPLE;
    } else if (length == 2) {
        base_weight = WEIGHT_DOUBLE;
    } else {
        return 0; // Линия слишком короткая
    }
    
    // Корректировка по количеству открытых концов
    if (open_ends == 0) {
        // Оба конца закрыты - линия бесполезна
        return 0;
    } else if (open_ends == 1) {
        // Один конец закрыт - вес делится пополам
        return base_weight / 2;
    } else {
        // Оба конца открыты - полный вес
        return base_weight;
    }
}
bool MyBot::is_in_bounds(const State& state, int x, int y) const {
    int rows = state.get_opts().rows;
    int cols = state.get_opts().cols;
    return (x >= 0 && x < cols && y >= 0 && y < rows);
}

int MyBot::count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int length = 0;
    
    int cx = x + dx;
    int cy = y + dy;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        length++;
        cx += dx;
        cy += dy;
    }

    int cx = x - dx;
    int cy = y - dy;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        length++;
        cx -= dx;
        cy -= dy;
    }
    
    return length;
}

int MyBot::count_open_ends(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int open_ends = 0;
    
    // Проверяем конец в направлении (dx, dy)
    int cx = x;
    int cy = y;
    // Идём до конца линии
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        cx += dx;
        cy += dy;
    }
    // Если следующая клетка пустая или за полем → конец открыт
    if (!is_in_bounds(state, cx, cy) || state.get_value(cx, cy) == Sign::NONE) {
        open_ends++;
    }
    
    // Проверяем конец в направлении (-dx, -dy)
    cx = x;
    cy = y;
    while (is_in_bounds(state, cx, cy) && state.get_value(cx, cy) == sign) {
        cx -= dx;
        cy -= dy;
    }
    if (!is_in_bounds(state, cx, cy) || state.get_value(cx, cy) == Sign::NONE) {
        open_ends++;
    }
    
    return open_ends;
}

Point MyBot::make_move(const State& state) {
    int rows = state.get_opts().rows;
    int cols = state.get_opts().cols;
    
    const int MAX_CELLS = 400;
    
    Point free_cells[MAX_CELLS];
    int free_count = 0;
    
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (state.get_value(x, y) == Sign::NONE) {
                free_cells[free_count].x = x;
                free_cells[free_count].y = y;
                free_count++;
            }
        }
    }
    
    if (free_count == 0) {
        return {0, 0};
    }
    
    int random_index = std::rand() % free_count;
    
    return free_cells[random_index];
}

}