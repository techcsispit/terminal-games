#include <iostream>
#include <limits>

#include "blackjack.h"
#include "dino.h"
#include "game2048.h"
#include "hangman.h"
#include "snake.h"
#include "tictactoe.h"
#include "minesweeper.h"

int main() {
    while (true) {
        std::cout << "\n=== Terminal Games ===\n"
                  << "1. 2048\n"
                  << "2. Tic-tac-toe (vs computer)\n"
                  << "3. Hangman\n"
                  << "4. Minesweeper\n"
                  << "5. Snake\n"
                  << "6. Dinosaur Run\n"
                  << "7. Blackjack\n"
                  << "8. Quit\n"
                  << "Pick a game: ";
        int choice = 0;

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                std::cout << "\nBye!\n";
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
            case 4: playMinesweeper(); break;
            case 5: playSnake(); break;
            case 6: playDino(); break;
            case 7: playBlackjack(); break;
            case 8: std::cout << "Bye!\n"; return 0;
            default: std::cout << "Please pick a number from 1 to 8.\n";
        }
    }
}
