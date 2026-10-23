#include "minesweeper.h"
#include <iostream>
#include <random>
#include <algorithm>

Minesweeper::Minesweeper() : Minesweeper(std::random_device{}()) {}

Minesweeper::Minesweeper(unsigned seed) : mine_(), revealed_(), flagged_(), seed_(seed), hasSeed_(true), rng_(seed) {
    clear();
    placeMines();
}

Minesweeper::Minesweeper(const std::vector<std::pair<int, int>>& fixedMines) : mine_(), revealed_(), flagged_(), hasSeed_(false), rng_(std::random_device{}()) {
    clear();
    for (const auto& p : fixedMines) {
        if (inBounds(p.first, p.second)) mine_[p.first][p.second] = true;
    }
}

void Minesweeper::clear() {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            mine_[r][c] = false;
            revealed_[r][c] = false;
            flagged_[r][c] = false;
        }
    }
    lost_ = false;
    firstReveal_ = true;
}

void Minesweeper::placeMines() {
    std::vector<std::pair<int, int>> cells;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            cells.push_back({r, c});
    std::shuffle(cells.begin(), cells.end(), rng_);
    for (int i = 0; i < MINES; ++i) {
        mine_[cells[i].first][cells[i].second] = true;
    }
}

bool Minesweeper::inBounds(int r, int c) const {
    return r >= 0 && r < ROWS && c >= 0 && c < COLS;
}

bool Minesweeper::isRevealed(int r, int c) const {
    return inBounds(r, c) && revealed_[r][c];
}

bool Minesweeper::isFlagged(int r, int c) const {
    return inBounds(r, c) && flagged_[r][c];
}

bool Minesweeper::isMine(int r, int c) const {
    return inBounds(r, c) && mine_[r][c];
}

int Minesweeper::adjacentMines(int r, int c) const {
    int count = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (inBounds(nr, nc) && mine_[nr][nc]) count++;
        }
    }
    return count;
}

bool Minesweeper::reveal(int r, int c) {
    if (!inBounds(r, c)) return false;
    if (revealed_[r][c]) return false;
    if (flagged_[r][c]) return false;

    if (firstReveal_) {
        firstReveal_ = false;
        if (mine_[r][c]) {
            // Move this mine to another random empty cell so the first reveal is safe.
            std::vector<std::pair<int, int>> safe;
            for (int rr = 0; rr < ROWS; ++rr) {
                for (int cc = 0; cc < COLS; ++cc) {
                    if (!mine_[rr][cc] && !(rr == r && cc == c)) safe.push_back({rr, cc});
                }
            }
            if (!safe.empty()) {
                std::uniform_int_distribution<size_t> dist(0, safe.size() - 1);
                size_t idx = dist(rng_);
                int mr = safe[idx].first;
                int mc = safe[idx].second;
                mine_[mr][mc] = true;
                mine_[r][c] = false;
            }
        }
    }

    revealed_[r][c] = true;
    if (mine_[r][c]) {
        lost_ = true;
        return false;
    }

    if (adjacentMines(r, c) == 0) {
        floodFill(r, c);
    }

    return true;
}

void Minesweeper::floodFill(int r, int c) {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (inBounds(nr, nc) && !revealed_[nr][nc] && !flagged_[nr][nc] && !mine_[nr][nc]) {
                revealed_[nr][nc] = true;
                if (adjacentMines(nr, nc) == 0) {
                    floodFill(nr, nc);
                }
            }
        }
    }
}

bool Minesweeper::toggleFlag(int r, int c) {
    if (!inBounds(r, c)) return false;
    if (revealed_[r][c]) return false;
    flagged_[r][c] = !flagged_[r][c];
    return true;
}

bool Minesweeper::won() const {
    int revealedSafe = 0;
    int totalMines = 0;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (mine_[r][c]) totalMines++;
            if (revealed_[r][c] && !mine_[r][c]) revealedSafe++;
        }
    }
    return revealedSafe == (ROWS * COLS - totalMines);
}

bool Minesweeper::lost() const { return lost_; }

std::string Minesweeper::boardStr() const {
    std::string s;
    int flags = 0;
    for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c) if (flagged_[r][c]) flags++;
    s += "========== MINESWEEPER ==========\n";
    s += "Board: 9x9 | Mines: " + std::to_string(MINES) + " | Flags: " + std::to_string(flags) + "\n\n";
    s += "       1   2   3   4   5   6   7   8   9\n";
    s += "    +-------------------------------------\n";
    for (int r = 0; r < ROWS; ++r) {
        s += " " + std::to_string(r + 1) + "  | ";
        for (int c = 0; c < COLS; ++c) {
            char ch = '?';
            if (revealed_[r][c]) {
                if (mine_[r][c]) ch = '*';
                else {
                    int n = adjacentMines(r, c);
                    ch = (n == 0 ? '.' : '0' + n);
                }
            } else if (flagged_[r][c]) {
                ch = 'F';
            }
            s += "[" + std::string(1, ch) + "]";
            if (c < COLS - 1) s += " ";
        }
        s += "\n";
    }
    s += "\nLegend: [?] Hidden  [F] Flag  [.] Empty  [1-8] Clue\n";
    return s;
}
void playMinesweeper() {
    Minesweeper game;
    std::cout << "\n=== Minesweeper ===\n"
              << "Board is 9x9 with 10 mines.\n"
              << "Commands: r row col (reveal), f row col (flag/unflag), q (quit)\n"
              << "Coordinates are 1-9 (top-left is row 1, column 1).\n";
    while (!game.won() && !game.lost()) {
        std::cout << "\n" << game.boardStr();
        std::cout << "Action (r/f) or q: ";
        char cmd = 0;
        if (!(std::cin >> cmd)) {
            if (std::cin.eof()) { std::cout << "\nExiting Minesweeper.\n"; return; }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (cmd == 'q' || cmd == 'Q') { std::cout << "Quitting Minesweeper.\n"; return; }
        if (cmd != 'r' && cmd != 'R' && cmd != 'f' && cmd != 'F') {
            std::cout << "Invalid command. Use r (reveal), f (flag), or q (quit).\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        int row = -1, col = -1;
        if (!(std::cin >> row >> col)) {
            if (std::cin.eof()) { std::cout << "\nExiting Minesweeper.\n"; return; }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Enter two numbers.\n";
            continue;
        }
        if (row < 1 || row > Minesweeper::ROWS || col < 1 || col > Minesweeper::COLS) {
            std::cout << "Invalid coordinates (use 1-9).\n"; continue;
        }
        row -= 1; col -= 1;
        if (cmd == 'r' || cmd == 'R') {
            if (!game.reveal(row, col)) {
                if (game.lost()) {
                    std::cout << "\nBOOM! You hit a mine.\n\nFinal board:\n" << game.boardStr() << "\nGame over!\n";
                } else {
                    std::cout << "That cell is already revealed or flagged.\n";
                }
            } else {
                if (game.won()) {
                    std::cout << "\nCongratulations! You cleared the board!\n" << game.boardStr();
                }
            }
        } else {
            game.toggleFlag(row, col);
        }
    }
}
