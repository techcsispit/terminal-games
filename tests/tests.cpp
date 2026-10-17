#include <algorithm>
#include <iostream>
#include <string>

#include "blackjack.h"
#include "dino.h"
#include "game2048.h"
#include "hangman.h"
#include "snake.h"
#include "tetris.h"
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
    resetMemo();
    CHECK(computerMove(board("XOX O X  "), 'O', 'X') == 7);
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

static void testBlackjack() {
    CHECK(Card(Suit::Hearts, Rank::Ace).value() == 11);
    CHECK(Card(Suit::Clubs, Rank::King).value() == 10);
    CHECK(Card(Suit::Diamonds, Rank::Seven).rankText() == "7");

    Hand soft;
    soft.add(Card(Suit::Spades, Rank::Ace));
    soft.add(Card(Suit::Hearts, Rank::Six));
    CHECK(soft.total() == 17);
    CHECK(soft.isSoft());
    soft.add(Card(Suit::Clubs, Rank::Nine));
    CHECK(soft.total() == 16);
    CHECK(!soft.isSoft());

    Hand natural;
    natural.add(Card(Suit::Spades, Rank::Ace));
    natural.add(Card(Suit::Diamonds, Rank::King));
    CHECK(natural.total() == 21);
    CHECK(natural.isBlackjack());

    Hand multiAce;
    multiAce.add(Card(Suit::Spades, Rank::Ace));
    multiAce.add(Card(Suit::Hearts, Rank::Ace));
    multiAce.add(Card(Suit::Clubs, Rank::Nine));
    CHECK(multiAce.total() == 21);
    CHECK(multiAce.isSoft());
    CHECK(!multiAce.isBlackjack());

    Deck shoe(42);
    CHECK(shoe.size() == 312);
    CHECK(shoe.remaining() == 312);
    std::array<std::array<int, 13>, 4> cardCounts{};
    for (int i = 0; i < 312; ++i) {
        const Card card = shoe.draw();
        ++cardCounts[static_cast<int>(card.suit())][static_cast<int>(card.rank()) - 2];
    }
    CHECK(shoe.remaining() == 0);
    for (int suit = 0; suit < 4; ++suit)
        for (int rank = 0; rank < 13; ++rank)
            CHECK(cardCounts[suit][rank] == 6);

    Deck penetration(42);
    for (int i = 0; i < 234; ++i) penetration.draw();
    CHECK(penetration.remaining() == 78);
    CHECK(!penetration.needsShuffle());
    penetration.draw();
    CHECK(penetration.remaining() == 77);
    CHECK(penetration.needsShuffle());
    penetration.prepareForRound();
    CHECK(penetration.remaining() == 312);
    CHECK(!penetration.needsShuffle());

    CHECK(BlackjackGame::returnFor(BlackjackResult::Blackjack, 1000) == 2500);
    CHECK(BlackjackGame::returnFor(BlackjackResult::PlayerWin, 1000) == 2000);
    CHECK(BlackjackGame::returnFor(BlackjackResult::Push, 1000) == 1000);
    CHECK(BlackjackGame::returnFor(BlackjackResult::DealerWin, 1000) == 0);

    BlackjackGame game(7);
    CHECK(game.bankrollCents() == BlackjackGame::kStartingBankrollCents);
    CHECK(!game.startRound(9));
    CHECK(!game.startRound(1001));
    CHECK(game.bankrollCents() == BlackjackGame::kStartingBankrollCents);
    CHECK(game.startRound(10));
    CHECK(game.wagerCents() == 1000);
    CHECK(game.bankrollCents() == 99000 || game.roundComplete());
    if (game.playerTurn()) {
        CHECK(game.canDoubleDown());
        CHECK(game.doubleDown());
        CHECK(game.wagerCents() == 2000);
        CHECK(!game.playerTurn());
        CHECK(!game.canDoubleDown());
        while (game.dealerTurn()) game.dealerStep();
        CHECK(game.roundComplete());
    }

    BlackjackGame allIn(9);
    CHECK(allIn.startRound(1000));
    if (allIn.playerTurn()) {
        CHECK(!allIn.canDoubleDown());
        CHECK(!allIn.doubleDown());
    }
}

static void testTetris() {
    TetrisGame first(1234);
    TetrisGame sameSeed(1234);
    std::vector<Tetromino> sequenceA;
    std::vector<Tetromino> sequenceB;
    for (int i = 0; i < 28; ++i) {
        sequenceA.push_back(first.currentPiece());
        sequenceB.push_back(sameSeed.currentPiece());
        CHECK(first.currentPiece() == sameSeed.currentPiece());
        first.hardDrop();
        sameSeed.hardDrop();
        first.setBoardForTesting(TetrisGame::Board{});
        sameSeed.setBoardForTesting(TetrisGame::Board{});
    }
    CHECK(sequenceA == sequenceB);
    for (int bag = 0; bag < 4; ++bag) {
        std::array<int, 7> counts{};
        for (int i = 0; i < 7; ++i) ++counts[static_cast<int>(sequenceA[bag * 7 + i])];
        for (int count : counts) CHECK(count == 1);
    }

    for (int piece = 0; piece < 7; ++piece) {
        TetrisGame rotating(42);
        const auto tetromino = static_cast<Tetromino>(piece);
        rotating.setCurrentForTesting(tetromino, 0, 3, 5);
        if (tetromino == Tetromino::O) {
            CHECK(!rotating.rotateClockwise());
            CHECK(rotating.rotation() == 0);
            continue;
        }
        for (int turn = 0; turn < 4; ++turn) CHECK(rotating.rotateClockwise());
        CHECK(rotating.rotation() == 0);
        for (int turn = 0; turn < 4; ++turn) CHECK(rotating.rotateCounterClockwise());
        CHECK(rotating.rotation() == 0);
    }

    TetrisGame wallKick(17);
    wallKick.setCurrentForTesting(Tetromino::I, 1, 7, 5);
    CHECK(!wallKick.isGameOver());
    CHECK(wallKick.rotateClockwise());
    CHECK(wallKick.rotation() == 2);
    CHECK(wallKick.x() == 6);

    TetrisGame boundaries(18);
    boundaries.setCurrentForTesting(Tetromino::T, 0, 0, 4);
    CHECK(!boundaries.moveLeft());
    boundaries.setCurrentForTesting(Tetromino::I, 0, 3, 0);
    TetrisGame::Board obstacle{};
    obstacle[5][4] = static_cast<int>(Tetromino::J) + 1;
    boundaries.setBoardForTesting(obstacle);
    boundaries.setCurrentForTesting(Tetromino::T, 0, 3, 3);
    CHECK(!boundaries.softDrop());
    CHECK(boundaries.y() == 3);
    boundaries.setBoardForTesting(TetrisGame::Board{});
    boundaries.setCurrentForTesting(Tetromino::T, 0, 3, 20);
    CHECK(!boundaries.softDrop());

    TetrisGame oneLine(21);
    TetrisGame::Board oneLineBoard{};
    for (int x = 0; x < 8; ++x) oneLineBoard[21][x] = 1;
    oneLine.setBoardForTesting(oneLineBoard);
    oneLine.setCurrentForTesting(Tetromino::O, 0, 7, 20);
    oneLine.hardDrop();
    CHECK(oneLine.lines() == 1);
    CHECK(oneLine.score() == 100);

    TetrisGame twoLines(22);
    TetrisGame::Board twoLineBoard{};
    for (int row = 20; row < 22; ++row)
        for (int x = 0; x < 8; ++x) twoLineBoard[row][x] = 1;
    twoLines.setBoardForTesting(twoLineBoard);
    twoLines.setCurrentForTesting(Tetromino::O, 0, 7, 20);
    twoLines.hardDrop();
    CHECK(twoLines.lines() == 2);
    CHECK(twoLines.score() == 300);

    TetrisGame tetris(23);
    TetrisGame::Board fourLineBoard{};
    for (int row = 18; row < 22; ++row)
        for (int x = 0; x < TetrisGame::WIDTH; ++x)
            if (x != 8) fourLineBoard[row][x] = 1;
    tetris.setBoardForTesting(fourLineBoard);
    tetris.setCurrentForTesting(Tetromino::I, 1, 6, 18);
    tetris.hardDrop();
    CHECK(tetris.lines() == 4);
    CHECK(tetris.score() == 800);
    fourLineBoard = tetris.board();
    for (int row = 18; row < 22; ++row) {
        fourLineBoard[row].fill(1);
        fourLineBoard[row][8] = 0;
    }
    tetris.setBoardForTesting(fourLineBoard);
    tetris.setCurrentForTesting(Tetromino::I, 1, 6, 18);
    tetris.hardDrop();
    CHECK(tetris.lines() == 8);
    CHECK(tetris.score() == 2000);

    TetrisGame drop(24);
    const int dropDistance = drop.ghostY() - drop.y();
    CHECK(dropDistance > 0);
    CHECK(drop.hardDrop() == dropDistance);
    CHECK(drop.score() == dropDistance * 2);

    TetrisGame softDrop(30);
    CHECK(softDrop.softDrop());
    CHECK(softDrop.y() == 1);
    CHECK(softDrop.score() == 1);

    TetrisGame gravity(31);
    CHECK(gravity.gravityPeriod() == 45);
    for (int tick = 0; tick < 44; ++tick) gravity.tick();
    CHECK(gravity.y() == 0);
    gravity.tick();
    CHECK(gravity.y() == 1);

    TetrisGame lockDelay(32);
    lockDelay.setCurrentForTesting(Tetromino::T, 0, 3, 20);
    for (int tick = 0; tick < TetrisGame::LOCK_DELAY_TICKS - 1; ++tick)
        lockDelay.tick();
    CHECK(lockDelay.board() == TetrisGame::Board{});
    lockDelay.tick();
    CHECK(lockDelay.board() != TetrisGame::Board{});

    TetrisGame lockReset(33);
    lockReset.setCurrentForTesting(Tetromino::T, 0, 3, 20);
    for (int tick = 0; tick < TetrisGame::LOCK_DELAY_TICKS - 1; ++tick)
        lockReset.tick();
    CHECK(lockReset.moveLeft());
    for (int tick = 0; tick < TetrisGame::LOCK_DELAY_TICKS - 1; ++tick)
        lockReset.tick();
    CHECK(lockReset.board() == TetrisGame::Board{});
    lockReset.tick();
    CHECK(lockReset.board() != TetrisGame::Board{});

    TetrisGame lockResetLimit(34);
    lockResetLimit.setCurrentForTesting(Tetromino::T, 0, 3, 20);
    for (int reset = 0; reset < TetrisGame::MAX_LOCK_RESETS; ++reset) {
        for (int tick = 0; tick < TetrisGame::LOCK_DELAY_TICKS - 1; ++tick)
            lockResetLimit.tick();
        CHECK(reset % 2 == 0 ? lockResetLimit.moveLeft() : lockResetLimit.moveRight());
    }
    for (int tick = 0; tick < TetrisGame::LOCK_DELAY_TICKS - 1; ++tick)
        lockResetLimit.tick();
    CHECK(lockResetLimit.board() == TetrisGame::Board{});
    CHECK(lockResetLimit.moveLeft());
    lockResetLimit.tick();
    CHECK(lockResetLimit.board() != TetrisGame::Board{});

    TetrisGame hold(25);
    const Tetromino outgoing = hold.currentPiece();
    CHECK(hold.canHold());
    CHECK(hold.hold());
    CHECK(hold.hasHeldPiece());
    CHECK(hold.heldPiece() == outgoing);
    CHECK(!hold.canHold());
    CHECK(!hold.hold());
    hold.hardDrop();
    CHECK(hold.canHold());

    TetrisGame ghost(26);
    TetrisGame::Board platform{};
    platform[21].fill(1);
    ghost.setBoardForTesting(platform);
    ghost.setCurrentForTesting(Tetromino::I, 0, 3, 0);
    CHECK(ghost.ghostY() == 19);

    TetrisGame progression(27);
    progression.setLinesForTesting(9);
    TetrisGame::Board levelBoard{};
    for (int x = 0; x < 8; ++x) levelBoard[21][x] = 1;
    progression.setBoardForTesting(levelBoard);
    progression.setCurrentForTesting(Tetromino::O, 0, 7, 20);
    progression.hardDrop();
    CHECK(progression.lines() == 10);
    CHECK(progression.level() == 2);
    CHECK(progression.gravityPeriod() < 60);
    progression.setLinesForTesting(140);
    CHECK(progression.level() == 15);
    CHECK(progression.gravityPeriod() == 3);

    TetrisGame topOut(28);
    TetrisGame::Board spawnBlocked{};
    for (auto& row : spawnBlocked) row.fill(1);
    topOut.setBoardForTesting(spawnBlocked);
    topOut.spawnNextForTesting();
    CHECK(topOut.isGameOver());

    TetrisGame bot(29);
    for (int tick = 0; tick < 5000; ++tick) {
        if (bot.isGameOver()) bot = TetrisGame(static_cast<std::uint32_t>(tick + 30));
        switch (tick % 7) {
            case 0: bot.moveLeft(); break;
            case 1: bot.moveRight(); break;
            case 2: bot.rotateClockwise(); break;
            case 3: bot.rotateCounterClockwise(); break;
            case 4: bot.softDrop(); break;
            case 5: bot.hold(); break;
            case 6: bot.tick(); break;
        }
        if (tick % 19 == 0) bot.hardDrop();
        if (!bot.isValidState()) {
            CHECK(false);
            break;
        }
    }
}

// A simple autopilot: jumps cacti and low birds, ducks mid-height birds.
static void autopilot(const DinoGame& game, bool& jump, bool& duck) {
    jump = false;
    duck = false;
    const double front = dino::kDinoX + dino::kDinoW;
    for (const auto& o : game.obstacles()) {
        if (o.x + o.width < dino::kDinoX + 4) continue;  // already behind us
        const double closing = game.speed() + o.speedOffset;
        const double d = o.x - front;
        const int altitude = dino::kGroundY - o.y - o.height;
        if (o.kind == DinoObstacle::Kind::Bird) {
            if (altitude >= dino::kBirdHigh) continue;
            if (altitude >= dino::kBirdMid) {
                duck = d <= closing * 14;
                return;
            }
        }
        const double launch = closing * DinoGame::kJumpTicks / 2.0 - (o.width + 10) / 2.0;
        jump = !game.airborne() && d <= launch;
        return;
    }
}

static void testDino() {
    CHECK(dinoArtIsValid());

    // Waits for the first jump.
    DinoGame ready(7);
    CHECK(ready.ready());
    for (int i = 0; i < 100; ++i) ready.step(false, false);
    CHECK(ready.ready());
    CHECK(ready.obstacles().empty());
    ready.step(true, false);
    CHECK(ready.running());
    CHECK(ready.airborne());

    // A jump peaks at the documented height and lands again after about a second.
    DinoGame jumper(7);
    jumper.step(true, false);
    int peak = jumper.heightAboveGround();
    int airTicks = 1;
    while (jumper.airborne() && airTicks < 200) {
        jumper.step(false, false);
        peak = std::max(peak, jumper.heightAboveGround());
        ++airTicks;
    }
    CHECK(peak >= 23 && peak <= 25);
    CHECK(airTicks >= 36 && airTicks <= 40);
    CHECK(!jumper.airborne());

    // Holding down in the air brings you back to the ground sooner.
    DinoGame diver(7);
    diver.step(true, false);
    for (int i = 0; i < 10; ++i) diver.step(false, false);
    int diveTicks = 0;
    while (diver.airborne() && diveTicks < 200) {
        diver.step(false, true);
        ++diveTicks;
    }
    CHECK(diveTicks < 10);

    // Standing still means crashing into the first obstacle, which is not at once.
    DinoGame idle(3);
    idle.step(true, false);
    long idleTicks = 0;
    while (!idle.crashed() && idleTicks < 2000) {
        idle.step(false, false);
        ++idleTicks;
    }
    CHECK(idle.crashed());
    CHECK(idleTicks > 100);
    CHECK(idle.highScore() == idle.score());
    const int frozenScore = idle.score();
    for (int i = 0; i < 50; ++i) idle.step(true, false);
    CHECK(idle.score() == frozenScore);  // a crashed game stays crashed

    // The same seed plays out the same way.
    DinoGame a(42), b(42);
    a.step(true, false);
    b.step(true, false);
    bool same = true;
    for (int i = 0; i < 1500; ++i) {
        a.step(false, false);
        b.step(false, false);
        if (a.crashed() != b.crashed() || a.score() != b.score() || a.obstacles().size() != b.obstacles().size())
            same = false;
    }
    CHECK(same);

    // Ducking helps against a mid-height bird, and only against that one.
    auto birdTest = [](int altitude, bool duck, bool jump) {
        DinoGame g(1, 100);
        g.start();
        g.spawnObstacle(DinoObstacle::Kind::Bird, 60, 1, altitude);
        for (int i = 0; i < 120 && !g.crashed(); ++i) {
            bool autoJump, autoDuck;
            autopilot(g, autoJump, autoDuck);
            g.step(jump && autoJump, duck);
        }
        return !g.crashed();
    };
    CHECK(!birdTest(dino::kBirdMid, false, false));  // standing: hit
    CHECK(birdTest(dino::kBirdMid, true, false));    // ducking: safe
    CHECK(!birdTest(dino::kBirdLow, true, false));   // ducking a low bird: hit
    CHECK(birdTest(dino::kBirdLow, false, true));    // jumping a low bird: safe
    CHECK(birdTest(dino::kBirdHigh, false, false));  // high bird: walk under
    CHECK(!birdTest(dino::kBirdLow, false, false));

    // A well-timed jump clears every kind of cactus group, at the start speed.
    auto cactusTest = [](DinoObstacle::Kind kind, int count) {
        DinoGame g(1, 100);
        g.start();
        g.spawnObstacle(kind, 100, count);
        for (int i = 0; i < 160 && !g.crashed(); ++i) {
            bool jump, duck;
            autopilot(g, jump, duck);
            g.step(jump, duck);
        }
        return !g.crashed();
    };
    CHECK(cactusTest(DinoObstacle::Kind::SmallCactus, 1));
    CHECK(cactusTest(DinoObstacle::Kind::SmallCactus, 2));
    CHECK(cactusTest(DinoObstacle::Kind::LargeCactus, 1));

    // The autopilot can play for a long time on several seeds. That shows the
    // generated obstacle gaps always leave enough room, even at full speed,
    // and that night, birds and score flashing all happen along the way.
    for (unsigned seed = 1; seed <= 6; ++seed) {
        DinoGame game(seed, 100);
        game.step(true, false);
        bool sawBird = false, sawNight = false, sawBigGroup = false;
        for (long i = 0; i < 40000 && !game.crashed(); ++i) {
            bool jump, duck;
            autopilot(game, jump, duck);
            game.step(jump, duck);
            for (const auto& o : game.obstacles()) {
                sawBird = sawBird || o.kind == DinoObstacle::Kind::Bird;
                sawBigGroup = sawBigGroup || o.count > 1;
            }
            sawNight = sawNight || game.night();
        }
        CHECK(!game.crashed());
        CHECK(game.score() > 2000);
        CHECK(game.speed() == DinoGame::kMaxSpeed);
        CHECK(sawBird && sawNight && sawBigGroup);
    }

    // The high score carries over to the next game.
    DinoGame next(5, 100, 1234);
    CHECK(next.highScore() == 1234);
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
    testBlackjack();
    testDino();
    testSnake();
    testTetris();
    if (failures == 0) std::cout << "All tests passed.\n";
    return failures == 0 ? 0 : 1;
}
