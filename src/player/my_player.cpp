#include "my_player.hpp"
#include <cstdlib>

namespace ttt::my_player {

MyPlayer::MyPlayer(const char *name) : m_sign(Sign::NONE), m_name(name) {}

void MyPlayer::set_sign(Sign sign) {
    m_sign = sign;
}
const char *MyPlayer::get_name() const {
    return m_name;
}

//система ценностей для цепочек из 5 клеток
int MyPlayer::evaluate_line(int my_count, int enemy_count) {
    //если в пяти клетках есть и наш знак, и знак соперника — линия заблокирована
    if (my_count > 0 && enemy_count > 0) return 0; 
    //если линия пустая
    if (my_count == 0 && enemy_count == 0) return 0;

    //оцениваем наши атаки (плюс в нашу пользу)
    if (my_count > 0) {
        if (my_count == 5) return 100000; // Победа!
        if (my_count == 4) return 10000;  // Шаг до победы
        if (my_count == 3) return 1000;   // Хорошая атака
        if (my_count == 2) return 100;
        return 10;
    }

    //оцениваем угрозы соперника (минус очки, чтобы бот избегал этих ходов)
    if (enemy_count > 0) {
        if (enemy_count == 5) return -100000; // Проигрыш
        if (enemy_count == 4) return -10000;  // Критическая угроза, нужно блокировать
        if (enemy_count == 3) return -1000;
        if (enemy_count == 2) return -100;
        return -10;
    }

    return 0;
}

//главная функция сканирования поля 20х20
int MyPlayer::evaluate_board(const State &state) {
    int total_score = 0;
    auto opts = state.get_opts();
    int rows = opts.rows;
    int cols = opts.cols;
    
    Sign enemy_sign = (m_sign == Sign::X) ? Sign::O : Sign::X;

    //1.сканируем горизонтали
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x <= cols - 5; ++x) {
            int my_count = 0, enemy_count = 0;
            for (int i = 0; i < 5; ++i) {
                Sign s = state.get_value(x + i, y);
                if (s == m_sign) my_count++;
                else if (s == enemy_sign) enemy_count++;
            }
            total_score += evaluate_line(my_count, enemy_count);
        }
    }

    //2.сканируем вертикали
    for (int x = 0; x < cols; ++x) {
        for (int y = 0; y <= rows - 5; ++y) {
            int my_count = 0, enemy_count = 0;
            for (int i = 0; i < 5; ++i) {
                Sign s = state.get_value(x, y + i);
                if (s == m_sign) my_count++;
                else if (s == enemy_sign) enemy_count++;
            }
            total_score += evaluate_line(my_count, enemy_count);
        }
    }

    //3.сканируем главные диагонали (сверху-слева направо-вниз)
    for (int y = 0; y <= rows - 5; ++y) {
        for (int x = 0; x <= cols - 5; ++x) {
            int my_count = 0, enemy_count = 0;
            for (int i = 0; i < 5; ++i) {
                Sign s = state.get_value(x + i, y + i);
                if (s == m_sign) my_count++;
                else if (s == enemy_sign) enemy_count++;
            }
            total_score += evaluate_line(my_count, enemy_count);
        }
    }

    //4.сканируем побочные диагонали (снизу-слева направо-вверх)
    for (int y = 4; y < rows; ++y) {
        for (int x = 0; x <= cols - 5; ++x) {
            int my_count = 0, enemy_count = 0;
            for (int i = 0; i < 5; ++i) {
                Sign s = state.get_value(x + i, y - i);
                if (s == m_sign) my_count++;
                else if (s == enemy_sign) enemy_count++;
            }
            total_score += evaluate_line(my_count, enemy_count);
        }
    }

    return total_score;
}

Point MyPlayer::make_move(const State &state) {
    auto opts = state.get_opts();
    for (int y = 0; y < opts.rows; ++y) {
        for (int x = 0; x < opts.cols; ++x) {
            if (state.get_value(x, y) == Sign::NONE) {
                return Point{x, y};
            }
        }
    }
    return Point{0, 0};
}

}; // namespace ttt::my_player
