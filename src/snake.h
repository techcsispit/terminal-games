#pragma once
#include <deque>
#include <random>

enum class SnakeDirection { Up, Down, Left, Right };

struct Point {
    int x = 0;
    int y = 0;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

class SnakeGame {
public:
    static constexpr int WIDTH = 20;
    static constexpr int HEIGHT = 10;

    explicit SnakeGame(unsigned seed = std::random_device{}());

    // Advance the snake by 1 step in the current direction.
    // Returns false if collision occurs (game over).
    bool step();

    // Change direction (ignored if opposite of current direction).
    void setDirection(SnakeDirection dir);

    SnakeDirection direction() const { return currentDir_; }
    bool isGameOver() const { return gameOver_; }
    bool hasWon() const { return won_; }
    int score() const { return score_; }

    const std::deque<Point>& body() const { return body_; }
    Point food() const { return food_; }

    // Helpers used for testing and deterministic setups
    void setFood(Point p) { food_ = p; }
    void setBody(const std::deque<Point>& b) { body_ = b; }

    void print() const;

private:
    void spawnFood();

    std::deque<Point> body_;  // Index 0 is the head, back is the tail
    SnakeDirection currentDir_ = SnakeDirection::Right;
    Point food_{};
    int score_ = 0;
    bool gameOver_ = false;
    bool won_ = false;
    std::mt19937 rng_;
};

void playSnake();
