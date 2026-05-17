#pragma once

#include "core/game.hpp"
#include <string>

namespace ttt::my_player {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

/**
 * @brief бот с эвристической оценкой позиций
 */
class MyBot : public IPlayer {
public:
    explicit MyBot(const char* name = "MyBot");
    ~MyBot() override = default;

    void set_sign(Sign sign) override;
    Point make_move(const State& state) override;
    const char* get_name() const override;

private:
    /**
     * @brief Считает длину непрерывной цепочки символов в заданном направлении
     * @param state Состояние игры
     * @param x Начальная координата X
     * @param y Начальная координата Y
     * @param dx Смещение по X (направление)
     * @param dy Смещение по Y (направление)
     * @param sign Символ для подсчета
     * @return Длина цепочки
     */
    int count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const;
    
    /**
     * @brief проверяет, находится ли клетка в пределах поля
     */
    bool is_in_bounds(const State& state, int x, int y) const;

    
    std::string m_name;
    Sign m_sign;
};

}