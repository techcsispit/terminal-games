#include "game2048.h"

#include <iomanip>
#include <iostream>
#include <vector>

Game2048::Game2048(unsigned seed) : rng_(seed) {
    addRandomTile();
    addRandomTile();
}

Row Game2048::slideRow(Row row, int& score) {
    // Push all non-zero tiles to the left, keeping their order.
    Row packed{};
    int count = 0;
    for (int value : row) {
        if (value != 0) packed[count++] = value;
    }

    // Merge adjacent equal tiles once per move.
    Row result{};
    int out = 0;
    for (int i = 0; i < count; i++) {
        if (i + 1 < count && packed[i] == packed[i + 1]) {
            int merged = packed[i] * 2;
            result[out++] = merged;
            score += merged;
            i++;  // Skip the second tile so it only merges once per move
        } else {
            result[out++] = packed[i];
        }
    }
    return result;
}

bool Game2048::move(Direction direction) {
    State stateBeforeMove{grid_, score_, rng_};
    bool moved = false;
    for (int line = 0; line < 4; line++) {
        // Read the line so that "forwards" is always towards index 0.
        Row row;
        for (int i = 0; i < 4; i++) {
            switch (direction) {
                case Direction::Left:  row[i] = grid_[line][i]; break;
                case Direction::Right: row[i] = grid_[line][3 - i]; break;
                case Direction::Up:    row[i] = grid_[i][line]; break;
                case Direction::Down:  row[i] = grid_[3 - i][line]; break;
            }
        }
        Row slid = slideRow(row, score_);
        if (slid != row) moved = true;
        for (int i = 0; i < 4; i++) {
            switch (direction) {
                case Direction::Left:  grid_[line][i] = slid[i]; break;
                case Direction::Right: grid_[line][3 - i] = slid[i]; break;
                case Direction::Up:    grid_[i][line] = slid[i]; break;
                case Direction::Down:  grid_[3 - i][line] = slid[i]; break;
            }
        }
    }
    if (moved) {
        previousState_ = stateBeforeMove;
        canUndo_ = true;
        addRandomTile();
    }
    return moved;
}

bool Game2048::undo() {
    if (!canUndo_) return false;
    grid_ = previousState_.grid;
    score_ = previousState_.score;
    rng_ = previousState_.rng;
    canUndo_ = false;
    return true;
}

bool Game2048::canMove() const {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (grid_[r][c] == 0) return true;
            if (c < 3 && grid_[r][c] == grid_[r][c + 1]) return true;
            if (r < 3 && grid_[r][c] == grid_[r + 1][c]) return true;
        }
    }
    return false;
}

bool Game2048::hasWon() const {
    for (const Row& row : grid_)
        for (int value : row)
            if (value >= 2048) return true;
    return false;
}

void Game2048::addRandomTile() {
    std::vector<std::pair<int, int>> empty;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (grid_[r][c] == 0) empty.push_back({r, c});
    if (empty.empty()) return;
    auto [r, c] = empty[std::uniform_int_distribution<size_t>(0, empty.size() - 1)(rng_)];
    grid_[r][c] = std::uniform_int_distribution<int>(1, 10)(rng_) == 10 ? 4 : 2;  // 10% chance of a 4
}

void Game2048::print() const {
    std::cout << "\nScore: " << score_ << "\n+------+------+------+------+\n";
    for (const Row& row : grid_) {
        std::cout << "|";
        for (int value : row) {
            if (value == 0) std::cout << "      |";
            else std::cout << std::setw(5) << value << " |";
        }
        std::cout << "\n+------+------+------+------+\n";
    }
}

void play2048() {
    Game2048 game;
    std::cout << "\nJoin the tiles to get to 2048! Moves: w a s d, u to undo, q to quit.\n";
    while (true) {
        game.print();
        if (game.hasWon()) { std::cout << "You made 2048!\n"; return; }
        if (!game.canMove()) { std::cout << "No moves left. Game over!\n"; return; }
        std::cout << "Move: ";
        char key;
        std::cin >> key;
        switch (key) {
            case 'a': game.move(Direction::Left); break;
            case 'd': game.move(Direction::Right); break;
            case 'w': game.move(Direction::Up); break;
            case 's': game.move(Direction::Down); break;
            case 'u':
                if (!game.undo()) std::cout << "Nothing to undo.\n";
                break;
            case 'q': return;
            default: std::cout << "Use w, a, s, d or u.\n";
        }
    }
}
