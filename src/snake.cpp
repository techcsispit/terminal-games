#include "snake.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

SnakeGame::SnakeGame(unsigned seed) : rng_(seed) {
    int startX = WIDTH / 2;
    int startY = HEIGHT / 2;
    body_.push_back({startX, startY});
    body_.push_back({startX - 1, startY});
    body_.push_back({startX - 2, startY});
    spawnFood();
}

void SnakeGame::setDirection(SnakeDirection dir) {
    if (currentDir_ == SnakeDirection::Up && dir == SnakeDirection::Down) return;
    if (currentDir_ == SnakeDirection::Down && dir == SnakeDirection::Up) return;
    if (currentDir_ == SnakeDirection::Left && dir == SnakeDirection::Right) return;
    if (currentDir_ == SnakeDirection::Right && dir == SnakeDirection::Left) return;
    currentDir_ = dir;
}

bool SnakeGame::step() {
    if (gameOver_) return false;

    Point newHead = body_.front();
    switch (currentDir_) {
        case SnakeDirection::Up:    newHead.y--; break;
        case SnakeDirection::Down:  newHead.y++; break;
        case SnakeDirection::Left:  newHead.x--; break;
        case SnakeDirection::Right: newHead.x++; break;
    }

    // Wrap around boundaries on all 4 sides
    if (newHead.x < 1) newHead.x = WIDTH - 2;
    else if (newHead.x > WIDTH - 2) newHead.x = 1;

    if (newHead.y < 1) newHead.y = HEIGHT - 2;
    else if (newHead.y > HEIGHT - 2) newHead.y = 1;

    // Self-collision (tail will vacate unless snake is eating food)
    size_t checkCount = body_.size() - (newHead == food_ ? 0 : 1);
    for (size_t i = 0; i < checkCount; ++i) {
        if (body_[i] == newHead) {
            gameOver_ = true;
            return false;
        }
    }

    if (newHead == food_) {
        body_.push_front(newHead);
        score_ += 10;
        if (body_.size() == static_cast<size_t>((WIDTH - 2) * (HEIGHT - 2))) {
            won_ = true;
            return true;
        }
        spawnFood();
    } else {
        body_.push_front(newHead);
        body_.pop_back();
    }

    return true;
}

void SnakeGame::spawnFood() {
    std::vector<Point> empty;
    for (int y = 1; y < HEIGHT - 1; ++y) {
        for (int x = 1; x < WIDTH - 1; ++x) {
            Point p{x, y};
            bool occupied = false;
            for (const auto& seg : body_) {
                if (seg == p) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) empty.push_back(p);
        }
    }

    if (empty.empty()) {
        won_ = true;
        return;
    }

    std::uniform_int_distribution<size_t> dist(0, empty.size() - 1);
    food_ = empty[dist(rng_)];
}

void SnakeGame::print() const {
    std::string out;
    out.reserve(512);
    out += "\nScore: " + std::to_string(score_) + "\n+";
    for (int x = 1; x < WIDTH - 1; ++x) out += '-';
    out += "+\n";

    for (int y = 1; y < HEIGHT - 1; ++y) {
        out += '|';
        for (int x = 1; x < WIDTH - 1; ++x) {
            Point p{x, y};
            if (p == body_.front()) {
                out += '@';
            } else {
                bool isBody = false;
                for (size_t i = 1; i < body_.size(); ++i) {
                    if (body_[i] == p) {
                        isBody = true;
                        break;
                    }
                }
                if (isBody) {
                    out += 'o';
                } else if (p == food_) {
                    out += '*';
                } else {
                    out += ' ';
                }
            }
        }
        out += "|\n";
    }

    out += '+';
    for (int x = 1; x < WIDTH - 1; ++x) out += '-';
    out += "+\n";

    std::cout << out;
}

#ifndef _WIN32
namespace {
struct TermSetup {
    struct termios orig_termios;
    TermSetup() {
        tcgetattr(STDIN_FILENO, &orig_termios);
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    ~TermSetup() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
};

bool kbhit_posix() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}
}  // namespace
#endif

namespace {
struct CursorRestorer {
    ~CursorRestorer() {
        std::cout << "\033[?25h" << std::flush;
    }
};
}  // namespace

void playSnake() {
    SnakeGame game;

    // Clear screen once at start and hide cursor to avoid flicker
    std::cout << "\033[2J\033[?25l" << std::flush;
    CursorRestorer restorer;

#ifndef _WIN32
    TermSetup term;
#endif

    std::deque<SnakeDirection> inputQueue;

    auto pushDirection = [&](SnakeDirection newDir) {
        SnakeDirection lastDir = inputQueue.empty() ? game.direction() : inputQueue.back();
        bool isOpposite = (lastDir == SnakeDirection::Up && newDir == SnakeDirection::Down) ||
                          (lastDir == SnakeDirection::Down && newDir == SnakeDirection::Up) ||
                          (lastDir == SnakeDirection::Left && newDir == SnakeDirection::Right) ||
                          (lastDir == SnakeDirection::Right && newDir == SnakeDirection::Left);
        if (!isOpposite && lastDir != newDir && inputQueue.size() < 2) {
            inputQueue.push_back(newDir);
        }
    };

    while (!game.isGameOver() && !game.hasWon()) {
        // Move cursor to top-left home without blanking screen
        std::cout << "\033[H";
        game.print();
        std::cout << "Controls: Arrow Keys or W A S D to steer, Q to quit\n" << std::flush;

#ifdef _WIN32
        while (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) {
                int arrow = _getch();
                if (arrow == 72) pushDirection(SnakeDirection::Up);
                else if (arrow == 80) pushDirection(SnakeDirection::Down);
                else if (arrow == 75) pushDirection(SnakeDirection::Left);
                else if (arrow == 77) pushDirection(SnakeDirection::Right);
            } else {
                char k = static_cast<char>(std::tolower(ch));
                if (k == 'q') {
                    std::cout << "\nExiting Snake...\n";
                    return;
                }
                if (k == 'w') pushDirection(SnakeDirection::Up);
                else if (k == 's') pushDirection(SnakeDirection::Down);
                else if (k == 'a') pushDirection(SnakeDirection::Left);
                else if (k == 'd') pushDirection(SnakeDirection::Right);
            }
        }
#else
        while (kbhit_posix()) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) == 1) {
                if (ch == '\033') {
                    char seq[2];
                    if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                        if (seq[0] == '[') {
                            if (seq[1] == 'A') pushDirection(SnakeDirection::Up);
                            else if (seq[1] == 'B') pushDirection(SnakeDirection::Down);
                            else if (seq[1] == 'D') pushDirection(SnakeDirection::Left);
                            else if (seq[1] == 'C') pushDirection(SnakeDirection::Right);
                        }
                    }
                } else {
                    char k = static_cast<char>(std::tolower(ch));
                    if (k == 'q') {
                        std::cout << "\nExiting Snake...\n";
                        return;
                    }
                    if (k == 'w') pushDirection(SnakeDirection::Up);
                    else if (k == 's') pushDirection(SnakeDirection::Down);
                    else if (k == 'a') pushDirection(SnakeDirection::Left);
                    else if (k == 'd') pushDirection(SnakeDirection::Right);
                }
            }
        }
#endif

        if (!inputQueue.empty()) {
            game.setDirection(inputQueue.front());
            inputQueue.pop_front();
        }

        game.step();
        std::this_thread::sleep_for(std::chrono::milliseconds(220));
    }

    std::cout << "\033[H";
    game.print();
    if (game.hasWon()) {
        std::cout << "\nCongratulations, you won!\n";
    } else {
        std::cout << "\nGame Over! Final Score: " << game.score() << "\n";
    }
}
