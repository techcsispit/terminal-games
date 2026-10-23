#include "minesweeper.h"
#include <iostream>
#include <string>
#include <vector>
int testFixedLayoutFloodFill() {
    std::vector<std::pair<int,int>> mines = {{0,0}};
    Minesweeper game(mines);
    bool ok = game.reveal(2, 2);
    if (!ok) { std::cout << "FAIL fixedFloodFill reveal\n"; return 1; }
    if (!game.isRevealed(2,2)) { std::cout << "FAIL fixedFloodFill (2,2) not revealed\n"; return 1; }
    if (game.isMine(2,2)) { std::cout << "FAIL fixedFloodFill (2,2) is mine\n"; return 1; }
    return 0;
}
int testFixedLayoutWin() {
    std::vector<std::pair<int,int>> mines = {{8,8}};
    Minesweeper game(mines);
    game.reveal(0,0);
    for (int r = 0; r < Minesweeper::ROWS; ++r) {
        for (int c = 0; c < Minesweeper::COLS; ++c) {
            if (!(r == 8 && c == 8) && !game.isRevealed(r,c) && !game.isMine(r,c)) {
                game.reveal(r, c);
            }
        }
    }
    if (!game.won()) { std::cout << "FAIL fixedWin\n"; return 1; }
    if (game.lost()) { std::cout << "FAIL fixedWin lost\n"; return 1; }
    return 0;
}
int testFixedLayoutLoss() {
    std::vector<std::pair<int,int>> mines = {{2,2}};
    Minesweeper game(mines);
    game.reveal(0,0);
    bool ok = game.reveal(2,2);
    if (ok) { std::cout << "FAIL fixedLoss reveal mine returned true\n"; return 1; }
    if (!game.lost()) { std::cout << "FAIL fixedLoss not lost\n"; return 1; }
    return 0;
}
int testFlagToggle() {
    Minesweeper game;
    game.toggleFlag(0,0);
    if (!game.isFlagged(0,0)) { std::cout << "FAIL flagSet\n"; return 1; }
    game.toggleFlag(0,0);
    if (game.isFlagged(0,0)) { std::cout << "FAIL flagClear\n"; return 1; }
    return 0;
}
int testFirstRevealSafety() {
    Minesweeper game(111);
    bool ok = game.reveal(0,0);
    if (game.lost()) { std::cout << "FAIL firstReveal lost\n"; return 1; }
    if (!ok) { std::cout << "FAIL firstReveal false\n"; return 1; }
    return 0;
}
int testSeedingRepeatable() {
    Minesweeper a(42), b(42);
    for (int r = 0; r < Minesweeper::ROWS; ++r)
        for (int c = 0; c < Minesweeper::COLS; ++c)
            if (a.isMine(r,c) != b.isMine(r,c)) { std::cout << "FAIL seedLayoutCell\n"; return 1; }
    a.reveal(4,4); b.reveal(4,4);
    if (a.isRevealed(4,4) != b.isRevealed(4,4)) { std::cout << "FAIL seedRepeat\n"; return 1; }
    // First-click relocation must also be identical for identical seeds
    if (a.lost() != b.lost()) { std::cout << "FAIL seedRelocationLost\n"; return 1; }
    return 0;
}
int testDifferentSeedsCanDiffer() {
    Minesweeper a(1), b(2);
    int ca=0, cb=0;
    for (int r=0;r<Minesweeper::ROWS;++r) for (int c=0;c<Minesweeper::COLS;++c) {
        if (a.isMine(r,c)) ca++;
        if (b.isMine(r,c)) cb++;
    }
    if (ca != Minesweeper::MINES || cb != Minesweeper::MINES) { std::cout << "FAIL seedDiff\n"; return 1; }
    return 0;
}
int testStoredEngineRepeatable() {
    Minesweeper a(42), b(42);
    for (int r = 0; r < Minesweeper::ROWS; ++r)
        for (int c = 0; c < Minesweeper::COLS; ++c)
            if (a.isMine(r, c) != b.isMine(r, c)) { std::cout << "FAIL engineRepeat mine\n"; return 1; }
    a.reveal(4,4); b.reveal(4,4);
    if (a.isRevealed(4,4) != b.isRevealed(4,4)) { std::cout << "FAIL engineRepeat reveal\n"; return 1; }
    return 0;
}
int main() {
    int f = 0;
    f += testFixedLayoutFloodFill();
    f += testFixedLayoutWin();
    f += testFixedLayoutLoss();
    f += testFlagToggle();
    f += testFirstRevealSafety();
    f += testSeedingRepeatable();
    f += testDifferentSeedsCanDiffer();
    f += testStoredEngineRepeatable();
    if (f == 0) std::cout << "All Minesweeper tests passed.\n";
    else std::cout << "Some Minesweeper tests failed (" << f << ").\n";
    return f;
}
