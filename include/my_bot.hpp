#pragma once

#include "core/game.hpp"
#include <string>

namespace ttt::my_player {

using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

struct SearchZone {
    int x_start, x_end, y_start, y_end;
};

/**
 * @brief Структура для хранения кандидата на ход
 */
struct Candidate {
    int x;
    int y;
    int score;  // Оценка кандидата (для будущего использования)
};

class MyBot : public IPlayer {
public:
    explicit MyBot(const char* name = "MyBot");
    ~MyBot() override = default;

    void set_sign(Sign sign) override;
    Point make_move(const State& state) override;
    const char* get_name() const override;

private:
    // Зона поиска
    Point find_last_move(const State& state) const;
    SearchZone get_search_zone(const State& state) const;

    /**
     * @brief Собрать всех кандидатов на ход в зоне поиска
     * @param state Состояние игры
     * @param zone Зона поиска
     * @return Массив кандидатов
     */
    int collect_candidates(const State& state, const SearchZone& zone, Candidate* candidates) const;
    
    /**
     * @brief Проверить, является ли клетка легальным ходом
     */
    bool is_valid_move(const State& state, int x, int y) const;
    
    // Веса
    int get_line_weight(int length, int open_ends) const;
    static const int WEIGHT_DOUBLE = 1;
    static const int WEIGHT_TRIPLE = 10;
    static const int WEIGHT_QUADRUPLE = 100;
    static const int WEIGHT_WIN = 10000;
    
    // Сканирование (каждый метод объявлен ровно 1 раз)
    int count_line(const State& state, int x, int y, int dx, int dy, Sign sign) const;
    int count_open_ends(const State& state, int x, int y, int dx, int dy, Sign sign) const;
    bool is_in_bounds(const State& state, int x, int y) const;
    
    std::string m_name;
    Sign m_sign;
};

}