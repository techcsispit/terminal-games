#include "tictactoe.h"

#include <iostream>

namespace {

const int LINES[][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // columns
    {0, 4, 8}, {2, 4, 6},              // diagonals
};

// A square where `player` would complete a line, or -1.
int winningSquare(const Board& board, char player) {
    for (int square = 0; square < 9; square++) {
        if (board[square] != ' ') continue;
        Board next = board;
        next[square] = player;
        if (winner(next) == player) return square;
    }
    return -1;
}

void show(const Board& board) {
    for (int row = 0; row < 3; row++) {
        std::cout << " " << board[row * 3] << " | " << board[row * 3 + 1] << " | " << board[row * 3 + 2] << "\n";
        if (row < 2) std::cout << "---+---+---\n";
    }
}

}  // namespace

char winner(const Board& board) {
    for (const auto& line : LINES) {
        char a = board[line[0]];
        if (a != ' ' && a == board[line[1]] && a == board[line[2]]) return a;
    }
    return ' ';
}

bool isFull(const Board& board) {
    for (char c : board)
        if (c == ' ') return false;
    return true;
}

int computerMove(const Board& board, char me, char opponent) {
    int square = winningSquare(board, me);  // win
    if (square != -1) return square;
    square = winningSquare(board, opponent);  // block
    if (square != -1) return square;
    for (int preferred : {4, 0, 2, 6, 8, 1, 3, 5, 7})  // centre, corners, sides
        if (board[preferred] == ' ') return preferred;
    return -1;
}

void playTicTacToe() {
    Board board;
    board.fill(' ');
    std::cout << "\nYou are X, the computer is O. Squares are numbered 1-9.\n\n";
    while (true) {
        show(board);
        int square;
        std::cout << "\nYour move (1-9): ";
        std::cin >> square;
        if (square < 1 || square > 9 || board[square - 1] != ' ') {
            std::cout << "Pick an empty square from 1 to 9.\n";
            continue;
        }
        board[square - 1] = 'X';
        if (winner(board) == 'X') { show(board); std::cout << "\nYou win!\n"; return; }
        if (isFull(board)) { show(board); std::cout << "\nIt's a draw!\n"; return; }

        board[computerMove(board, 'O', 'X')] = 'O';
        if (winner(board) == 'O') { show(board); std::cout << "\nThe computer wins!\n"; return; }
    }
}
