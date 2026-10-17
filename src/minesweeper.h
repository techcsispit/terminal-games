#pragma once
#include <array>
#include <random>
#include <string>
#include <vector>

class Minesweeper {
public:
    static const int ROWS = 9;
    static const int COLS = 9;
    static const int MINES = 10;

    Minesweeper();                              // default RNG seeded
    Minesweeper(unsigned seed);                 // deterministic RNG seeded
    Minesweeper(const std::vector<std::pair<int, int>>& fixedMines);  // fixed layout for testing

    // Reveal a cell at (r,c). Returns false if invalid/already revealed/flagged/mined.
    bool reveal(int r, int c);

    // Toggle flag on a hidden cell.
    bool toggleFlag(int r, int c);

    bool isRevealed(int r, int c) const;
    bool isFlagged(int r, int c) const;
    bool isMine(int r, int c) const;
    int adjacentMines(int r, int c) const;

    bool won() const;
    bool lost() const;

    std::string boardStr() const; // simple text representation for tests

private:
    std::array<std::array<bool, COLS>, ROWS> mine_{};
    std::array<std::array<bool, COLS>, ROWS> revealed_{};
    std::array<std::array<bool, COLS>, ROWS> flagged_{};
    bool lost_ = false;
    bool firstReveal_ = true;
    unsigned seed_ = 0;
    bool hasSeed_ = false;
    std::mt19937 rng_;

    void clear();
    void placeMines();
    void floodFill(int r, int c);
    bool inBounds(int r, int c) const;
};

void playMinesweeper();
