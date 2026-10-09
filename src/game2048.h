#pragma once
#include <array>
#include <random>

enum class Direction { Left, Right, Up, Down };

using Row = std::array<int, 4>;

class Game2048 {
public:
    explicit Game2048(unsigned seed = std::random_device{}());

    // Slide and merge one row to the left. Adds merged tile values to `score`.
    static Row slideRow(Row row, int& score);

    bool move(Direction direction);  // true if any tile moved
    bool undo();                     // restore the state before the last successful move
    bool canMove() const;
    bool hasWon() const;             // a 2048 tile exists
    int score() const { return score_; }
    const std::array<Row, 4>& grid() const { return grid_; }
    void setGrid(const std::array<Row, 4>& grid) { grid_ = grid; }
    void print() const;

private:
    struct State {
        std::array<Row, 4> grid;
        int score;
        std::mt19937 rng;
    };

    void addRandomTile();

    std::array<Row, 4> grid_{};
    int score_ = 0;
    std::mt19937 rng_;
    State previousState_{};
    bool canUndo_ = false;
};

void play2048();
