#include <iostream>
#include <string>

#include "game2048.h"
#include "hangman.h"
#include "snake.h"
#include "tictactoe.h"

static int failures = 0;

#define CHECK(expr)                                                              \
    do {                                                                         \
        if (!(expr)) {                                                           \
            std::cout << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #expr "\n"; \
            failures++;                                                          \
        }                                                                        \
    } while (0)

static Board board(const std::string& s) {
    Board b;
    for (int i = 0; i < 9; i++) b[i] = s[i];
    return b;
}

// Plays every possible sequence of X (player) moves, O (computer) replies with
// computerMove each time. Returns false if X ever wins or O makes an illegal move.
static int gamesPlayed = 0;

static bool computerNeverLoses(const Board& b) {
    for (int i = 0; i < 9; i++) {
        if (b[i] != ' ') continue;
        Board next = b;
        next[i] = 'X';
        if (winner(next) == 'X') return false;
        if (isFull(next)) { gamesPlayed++; continue; }

        int m = computerMove(next, 'O', 'X');
        if (m < 0 || m > 8 || next[m] != ' ') return false;
        next[m] = 'O';
        if (winner(next) == 'O' || isFull(next)) { gamesPlayed++; continue; }

        if (!computerNeverLoses(next)) return false;
    }
    return true;
}

static void testTicTacToe() {
    resetMemo();  
    CHECK(winner(board("XXX OO   ")) == 'X');
    CHECK(winner(board("O  O  O  ")) == 'O');
    CHECK(winner(board("X   X   X")) == 'X');
    CHECK(winner(board("  X X X  ")) == 'X');
    CHECK(winner(board("XOXOXOOXO")) == ' ');
    CHECK(isFull(board("XOXOXOOXO")));
    CHECK(computerMove(board("OO XX    "), 'O', 'X') == 2);  // takes the win
    CHECK(computerMove(board("XX  O    "), 'O', 'X') == 2);  // blocks (only non-losing move)
    CHECK(computerMove(board("         "), 'O', 'X') == 4);  // centre first

    resetMemo();  // boards above aren't all legal positions; start the memo clean
    gamesPlayed = 0;
    CHECK(computerNeverLoses(board("         ")));  // X moves first, every line of play
    CHECK(gamesPlayed > 0);                          // check that atleast 1 game is played and test done
}

static void test2048() {
    int score = 0;
    CHECK((Game2048::slideRow({2, 2, 0, 0}, score) == Row{4, 0, 0, 0}));
    CHECK(score == 4);
    CHECK((Game2048::slideRow({0, 0, 0, 2}, score) == Row{2, 0, 0, 0}));
    CHECK((Game2048::slideRow({2, 4, 8, 16}, score) == Row{2, 4, 8, 16}));

    int score_quad = 0;
    CHECK((Game2048::slideRow({2, 2, 2, 2}, score_quad) == Row{4, 4, 0, 0}));
    CHECK(score_quad == 8);

    int score_cascade = 0;
    CHECK((Game2048::slideRow({4, 2, 2, 0}, score_cascade) == Row{4, 4, 0, 0}));
    CHECK(score_cascade == 4);

    int score_pairs = 0;
    CHECK((Game2048::slideRow({2, 2, 4, 4}, score_pairs) == Row{4, 8, 0, 0}));
    CHECK(score_pairs == 12);

    Game2048 game(42);
    game.setGrid({Row{2, 2, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}});
    CHECK(game.move(Direction::Right));
    CHECK(game.grid()[0][3] == 4);
    CHECK(!game.hasWon());

    Game2048 noOpGame(42);
    noOpGame.setGrid({Row{2, 0, 0, 0}, Row{4, 0, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}});
    auto beforeNoOp = noOpGame.grid();
    CHECK(!noOpGame.move(Direction::Left));
    CHECK(noOpGame.grid() == beforeNoOp);
    CHECK(noOpGame.score() == 0);

    Game2048 undoGame(42);
    std::array<Row, 4> beforeUndo{Row{2, 2, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}};
    undoGame.setGrid(beforeUndo);
    CHECK(undoGame.move(Direction::Right));
    auto afterMove = undoGame.grid();
    CHECK(undoGame.score() == 4);
    CHECK(undoGame.undo());
    CHECK(undoGame.grid() == beforeUndo);
    CHECK(undoGame.score() == 0);
    CHECK(!undoGame.undo());  // only one undo is available
    CHECK(undoGame.move(Direction::Right));
    CHECK(undoGame.grid() == afterMove);  // RNG state was restored too

    Game2048 preservedUndoGame(42);
    std::array<Row, 4> beforePreservedUndo{
        Row{2, 2, 8, 16}, Row{32, 64, 128, 256},
        Row{64, 128, 256, 512}, Row{128, 256, 512, 1024}};
    preservedUndoGame.setGrid(beforePreservedUndo);
    CHECK(preservedUndoGame.move(Direction::Left));
    CHECK(!preservedUndoGame.move(Direction::Left));
    CHECK(preservedUndoGame.undo());
    CHECK(preservedUndoGame.grid() == beforePreservedUndo);
    CHECK(preservedUndoGame.score() == 0);

    Game2048 noUndoGame(42);
    noUndoGame.setGrid({Row{2, 0, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}, Row{0, 0, 0, 0}});
    CHECK(!noUndoGame.move(Direction::Left));
    CHECK(!noUndoGame.undo());

    game.setGrid({Row{2, 4, 2, 4}, Row{4, 2, 4, 2}, Row{2, 4, 2, 4}, Row{4, 2, 4, 2}});
    CHECK(!game.canMove());
}

static void testHangman() {
    Hangman game("github");
    CHECK(game.guess('g'));
    CHECK(!game.guess('z'));
    CHECK(game.wrongGuesses() == 1);
    CHECK(game.masked() == "g _ _ _ _ _");
    for (char c : std::string("ithub")) game.guess(c);
    CHECK(game.won());
}

static void testSnake() {
    SnakeGame game(42);
    CHECK(!game.isGameOver());
    CHECK(!game.hasWon());
    CHECK(game.score() == 0);
    CHECK(game.body().size() == 3);

    // Initial movement (default direction is Right)
    Point initialHead = game.body().front();
    CHECK(game.step());
    CHECK(game.body().front().x == initialHead.x + 1);
    CHECK(game.body().front().y == initialHead.y);
    CHECK(game.body().size() == 3);

    // Disallow reversing into itself
    game.setDirection(SnakeDirection::Left);
    CHECK(game.direction() == SnakeDirection::Right);

    // Turn Down
    game.setDirection(SnakeDirection::Down);
    CHECK(game.direction() == SnakeDirection::Down);
    CHECK(game.step());
    CHECK(game.body().front().y == initialHead.y + 1);

    // Test eating food and growing
    SnakeGame foodGame(42);
    Point foodHead = foodGame.body().front();
    foodGame.setFood({foodHead.x + 1, foodHead.y});
    CHECK(foodGame.step());
    CHECK(foodGame.score() == 10);
    CHECK(foodGame.body().size() == 4);
    CHECK((foodGame.body().front() == Point{foodHead.x + 1, foodHead.y}));

    // Test boundary wrap-around in all 4 directions
    // Right -> Left
    SnakeGame wrapRight(42);
    wrapRight.setBody({{SnakeGame::WIDTH - 2, 5}, {SnakeGame::WIDTH - 3, 5}, {SnakeGame::WIDTH - 4, 5}});
    wrapRight.setDirection(SnakeDirection::Right);
    CHECK(wrapRight.step());
    CHECK(wrapRight.body().front().x == 1);
    CHECK(!wrapRight.isGameOver());

    // Left -> Right
    SnakeGame wrapLeft(42);
    wrapLeft.setDirection(SnakeDirection::Up);
    wrapLeft.setBody({{1, 5}, {2, 5}, {3, 5}});
    wrapLeft.setDirection(SnakeDirection::Left);
    CHECK(wrapLeft.step());
    CHECK(wrapLeft.body().front().x == SnakeGame::WIDTH - 2);
    CHECK(!wrapLeft.isGameOver());

    // Up -> Bottom
    SnakeGame wrapUp(42);
    wrapUp.setDirection(SnakeDirection::Up);
    wrapUp.setBody({{5, 1}, {5, 2}, {5, 3}});
    CHECK(wrapUp.step());
    CHECK(wrapUp.body().front().y == SnakeGame::HEIGHT - 2);
    CHECK(!wrapUp.isGameOver());

    // Down -> Top
    SnakeGame wrapDown(42);
    wrapDown.setDirection(SnakeDirection::Down);
    wrapDown.setBody({{5, SnakeGame::HEIGHT - 2}, {5, SnakeGame::HEIGHT - 3}, {5, SnakeGame::HEIGHT - 4}});
    CHECK(wrapDown.step());
    CHECK(wrapDown.body().front().y == 1);
    CHECK(!wrapDown.isGameOver());

    // Test self-collision
    SnakeGame selfGame(42);
    selfGame.setDirection(SnakeDirection::Up);
    std::deque<Point> selfBody = {
        {5, 5}, {5, 6}, {4, 6}, {4, 5}, {4, 4}
    };
    selfGame.setBody(selfBody);
    selfGame.setDirection(SnakeDirection::Left);
    CHECK(!selfGame.step());
    CHECK(selfGame.isGameOver());
}

int main() {
    testTicTacToe();
    test2048();
    testHangman();
    testSnake();
    if (failures == 0) std::cout << "All tests passed.\n";
    return failures == 0 ? 0 : 1;
}
