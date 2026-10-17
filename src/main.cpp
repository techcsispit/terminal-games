#include <iostream>
#include <limits>

#include "blackjack.h"
#include "dino.h"
#include "game2048.h"
#include "hangman.h"
#include "snake.h"
#include "tetris.h"
#include "tictactoe.h"

int main() {
    while (true) {
        std::cout << "\n=== Terminal Games ===\n"
                  << "1. 2048\n"
                  << "2. Tic-tac-toe (vs computer)\n"
                  << "3. Hangman\n"
                  << "4. Snake\n"
                  << "5. Dinosaur Run\n"
                  << "6. Blackjack\n"
                  << "7. Tetris\n"
                  << "8. Quit\n"
                  << "Pick a game: ";
        int choice = 0;

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                return 0;
            }

            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Please enter a valid number.\n";
            continue;
        }
        switch (choice) {
            case 1: play2048(); break;
            case 2: playTicTacToe(); break;
            case 3: playHangman(); break;
            case 4: playSnake(); break;
            case 5: playDino(); break;
            case 6: playBlackjack(); break;
            case 7: playTetris(); break;
            case 8: std::cout << "Bye!\n"; return 0;
            default: std::cout << "Please pick 1, 2, 3, 4, 5, 6, 7 or 8.\n";
        }
    }
}
