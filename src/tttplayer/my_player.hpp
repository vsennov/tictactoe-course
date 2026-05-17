#ifndef TTTPLAYER_MY_PLAYER_HPP
#define TTTPLAYER_MY_PLAYER_HPP

#include <string>
#include "../core/game.hpp"

namespace ttt::my_player {

class MyPlayer : public ttt::game::IPlayer {
public:
    // Конструктор теперь принимает имя, как в тесте
    MyPlayer(std::string name) : name_(std::move(name)) {}
    ~MyPlayer() override = default;

    // Методы интерфейса IPlayer
    void set_sign(ttt::game::Sign sign) override { sign_ = sign; }
    ttt::game::Point make_move(const ttt::game::State& state) override;
    const char* get_name() const override { return name_.c_str(); }

    // Метод интерфейса IObserver
    void handle_event(const ttt::game::State&, const ttt::game::Event&) override {}

private:
    ttt::game::Sign sign_ = ttt::game::Sign::NONE;
    std::string name_;
};

} // namespace ttt::my_player

#endif
