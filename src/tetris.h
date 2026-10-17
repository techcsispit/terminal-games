#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

enum class Tetromino { I, O, T, S, Z, J, L };

struct TetrisPoint {
    int x = 0;
    int y = 0;
};

class TetrisGame {
public:
    static constexpr int WIDTH = 10;
    static constexpr int VISIBLE_HEIGHT = 20;
    static constexpr int HIDDEN_ROWS = 2;
    static constexpr int HEIGHT = VISIBLE_HEIGHT + HIDDEN_ROWS;
    static constexpr int LOCK_DELAY_TICKS = 30;
    static constexpr int MAX_LOCK_RESETS = 15;
    using Board = std::array<std::array<int, WIDTH>, HEIGHT>;

    explicit TetrisGame(std::uint32_t seed = std::random_device{}());

    bool moveLeft();
    bool moveRight();
    bool softDrop();
    int hardDrop();
    bool rotateClockwise();
    bool rotateCounterClockwise();
    bool hold();
    void tick();

    const Board& board() const { return board_; }
    Tetromino currentPiece() const { return currentPiece_; }
    int rotation() const { return rotation_; }
    int x() const { return x_; }
    int y() const { return y_; }
    int ghostY() const;
    std::array<Tetromino, 3> nextPieces() const;
    bool hasHeldPiece() const { return heldPieceValid_; }
    Tetromino heldPiece() const { return heldPiece_; }
    bool canHold() const { return !holdUsed_ && !gameOver_; }
    bool isGameOver() const { return gameOver_; }
    int score() const { return score_; }
    int level() const { return lines_ / 10 + 1; }
    int lines() const { return lines_; }
    int gravityPeriod() const;
    bool isValidState() const;

    void setBoardForTesting(const Board& board);
    void setCurrentForTesting(Tetromino piece, int rotation, int x, int y);
    void setLinesForTesting(int lines);
    void spawnNextForTesting() { spawnNext(); }

private:
    void refillQueue();
    Tetromino drawPiece();
    bool canPlace(Tetromino piece, int rotation, int x, int y) const;
    bool move(int dx, int dy);
    bool rotate(int direction);
    void resetLockDelayIfNeeded(bool wasGrounded);
    bool isGrounded() const;
    void lockPiece();
    void clearLines();
    void spawnNext();
    void placeAtSpawn(Tetromino piece);

    Board board_{};
    std::mt19937 rng_;
    std::array<Tetromino, 7> bag_{};
    std::size_t bagIndex_ = bag_.size();
    std::vector<Tetromino> queue_;
    Tetromino currentPiece_ = Tetromino::I;
    Tetromino heldPiece_ = Tetromino::I;
    bool heldPieceValid_ = false;
    bool holdUsed_ = false;
    bool gameOver_ = false;
    int rotation_ = 0;
    int x_ = 3;
    int y_ = 0;
    int score_ = 0;
    int lines_ = 0;
    int gravityTicks_ = 0;
    int lockTicks_ = 0;
    int lockResets_ = 0;
    bool backToBackTetris_ = false;
};

void playTetris();
