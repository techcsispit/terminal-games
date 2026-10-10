#include "tictactoe.h"

#include <iostream>
#include <limits>
int dp[20000];

bool readInt(std::istream& input, int& value) {
    if (!(input >> value)) {
        input.clear();
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;
}

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

int state(const Board&board){
    int ans = 0;
    int mul = 1;
    for(int i=0;i<9;i++){
        int val;
        if(board[i]==' ') val = 0;
        if(board[i]=='O') val = 1;
        if(board[i]=='X') val = 2;
        ans+=val*mul;
        mul*=3;
    }
    return ans;
}

int max(int a,int b){
    if(a>b) return a;
    return b;
}

int chooseMove(Board board, char me, char opponent) {
    if(dp[state(board)]!=-2) return dp[state(board)];
    if(winner(board)==me){
        dp[state(board)]=1;
        return 1;
    }
    else if(winner(board)==opponent){
        dp[state(board)]=-1;
        return -1;
    }
    else if(isFull(board)){
        dp[state(board)]=0;
        return 0;
    }
    int best_score = -2;
    for(int i=0;i<9;i++){
        if(board[i]==' '){
            Board next = board;
            next[i]=me;
            int ret = -1*chooseMove(next,opponent,me);
            best_score = max(best_score,ret);
        }
    }
    dp[state(board)]=best_score;
    return best_score;
}

void resetMemo(){
    for(int i=0;i<20000;i++) dp[i]=-2;
}

int computerMove(const Board &board, char me, char opponent){
    // Tie-break order when moves score equally: centre, corners, edges.
    static const int ORDER[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    int move = -1;
    int best_score = -2;
    for(int i : ORDER){
        if(board[i]!=' ') continue;
        Board next = board;
        next[i]=me;
        int score = -chooseMove(next,opponent,me);  // chooseMove scores for the side to move
        if(score>best_score){
            best_score = score;
            move = i;
        }
    }
    return move;
}

void playTicTacToe() {
    resetMemo();
    Board board;
    board.fill(' ');
    std::cout << "\nYou are X, the computer is O. Squares are numbered 1-9.\n\n";
    while (true) {
        show(board);
        int square;
        std::cout << "\nYour move (1-9): ";
        if (!readInt(std::cin, square)) {
            std::cout << "Pick an empty square from 1 to 9.\n";
            continue;
        }
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
