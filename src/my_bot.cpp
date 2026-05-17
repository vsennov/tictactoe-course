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

bool MyBot::is_in_bounds(const State& state, int x, int y) const {
    int rows = state.get_opts().rows;
    int cols = state.get_opts().cols;
    return (x >= 0 && x < cols && y >= 0 && y < rows);
}

int MyBot::count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const {
    int length = 0;
    
    // Идем в направлении (dx, dy), пока видим свой символ
    while (is_in_bounds(state, x, y) && state.get_value(x, y) == sign) {
        length++;
        x += dx;
        y += dy;
    }
    
    return length;
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