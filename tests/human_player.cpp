#include "human_player.hpp"
#include "player/my_observer.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>


namespace ttt::human_player {

void HumanPlayer::set_sign(Sign sign) {
    m_sign = sign;
}

const char* HumanPlayer::get_name() const {
    return m_name.c_str();
}


Point HumanPlayer::make_move(const State& state) {
    //print the current board state
    my_player::ConsoleWriter::print_game_state(state);
    
    //get input from the user
    Point result;
    bool valid_input = false;
    
    while (!valid_input) {
        std::cout << "Player " << sign_to_char(m_sign) << ", enter your move (x y): ";
        
        std::string input;
        std::getline(std::cin, input);
        
        //Parse input
        std::istringstream iss(input);
        if (!(iss >> result.x >> result.y)) {
            std::cout << "Invalid input format. Please enter two numbers separated by space.\n";
            continue;
        }
        
        //move validity condition ()
        if (result.x < 0 || result.x >= state.get_opts().cols || 
            result.y < 0 || result.y >= state.get_opts().rows) {
            std::cout << "Move out of bounds. Try again.\n";
            continue;
        }
        
        if (state.get_value(result.x, result.y) != Sign::NONE) {
            std::cout << "That position is already occupied. Try again.\n";
            continue;
        }
        
        valid_input = true;
    }
    
    return result;
}

char HumanPlayer::sign_to_char(Sign sign) const {
    switch (sign) {
        case Sign::X: return 'X';
        case Sign::O: return 'O';
        default: return '.';
    }
}

} // namespace ttt::human_player
