# terminal-games

2048, tic-tac-toe against the computer, hangman, Snake, Minesweeper, Blackjack, Tetris, Space Invaders, and a dinosaur run in the terminal. Written in C++17.

## Running it

You need a C++ compiler and CMake.

```
cmake -B build
cmake --build build
./build/games
ctest --test-dir build --output-on-failure
```

Run `cmake --build build` again after every change.

## How it's supposed to work

Menu
- Choose a numbered game card, `0` to change menu page when available, or the displayed Quit option. In the current 8-game menu, Tetris is 7, Space Invaders is 8, and Quit is 9. Invalid entries print an error and ask again.

2048 (`w a s d` to move, `u` to undo)
- Tiles slide as far as they can. Equal tiles that meet merge, and a tile only merges once per move: `2 2 2 2` moved left is `4 4 _ _`.
- The score goes up by the value of each merged tile.
- A new tile appears after a move only if something actually moved.
- Undo restores the board, score, and random state from before the most recent successful move.
- The game ends when no move is possible.

Tic-tac-toe
- Three in a row wins: rows, columns, and both diagonals.
- The computer wins if it can, otherwise blocks you, otherwise takes the centre, then a corner.

Hangman
- Upper and lower case letters count as the same guess.
- A wrong guess costs a life, repeating a guess costs nothing. You get 6 lives.

Snake (arrow keys or `w a s d` to steer, `p` to pause, `q` to quit)
- Interactive retro title screen with animated falling snake cascade matrix.
- Arcade Mode: expanded textured arena, multiple food targets (`$`), session high-score tracking, and dynamic speed levels (1 to 9).
- Steer with arrow keys or `w` (up), `a` (left), `s` (down), `d` (right).
- The snake wraps around all boundaries (passing through one side brings you out the opposite side).
- The game ends if the snake runs into its own body.

Dinosaur Run (like Chrome's offline dinosaur game)
- Space or Up jumps, Down ducks, `q` returns to the menu. No Enter needed.
- Cacti and pterodactyls come at you and the game keeps getting faster. Hitting one ends the run.
- Jump low birds, duck under middle ones, and run under high ones.
- Space restarts after a crash. The high score is kept until you quit the program.

Blackjack
- Start with $1,000; wagers are $10 or more, up to your available bankroll.
- Hover over and click the $10, $25, $50, or $100 chips to build a wager; keys `1`–`4` work as a keyboard alternative. Backspace removes the last chip, `c` clears the bet, and Enter deals.
- Hover over and click Hit, Stand, or Double Down; press `H`, `S`, or `D` as a keyboard alternative. Double Down is available on the first two cards when you can match the wager. The dealer hits through 16 and stands on all 17s.
- A natural Blackjack pays 3:2; a regular win pays 1:1, and a tie returns the wager.
- Press `q` at the wager prompt or after a round to return to the menu.

Space Invaders (arrow keys or `a`/`d` to steer, Space or `w`/Up to shoot, `q` to quit)
- Defend against marching waves of alien invaders across 3 distinct tiers (Crimson, Green, and Purple).
- Use the 4 destructible bunkers for tactical cover; bunkers erode block-by-block when struck by player lasers or alien bombs.
- Player cannon (`/-^-\`) can fire up to 2 active lasers (`:`) on screen.
- Alien fleet shifts and descends whenever reaching the borders; movement speed accelerates as alien numbers diminish.
- Clear all invaders to advance to the next level. The game ends if all lives are lost or invaders descend to bunker/cannon level.

Tetris
- The playfield is 10 columns wide with 20 visible rows and 2 hidden spawn rows. Pieces come from a seeded 7-bag randomizer; each bag contains all seven tetrominoes once, and the same seed produces the same sequence.
- Left/Right arrows or `a`/`d` move; Down or `s` soft-drops one row for 1 point; Up or `x` rotates clockwise; `z` rotates counter-clockwise. Rotation uses SRS wall kicks. Controls work without pressing Enter, and each Down press soft-drops one row.
- Space hard-drops and locks the piece for 2 points per row. `c` holds or swaps the current piece once per turn; `p` pauses the game timers; `q`, Escape, or Ctrl-C returns to the menu.
- A ghost shows the landing position. The side panels display the held piece, next three pieces, score, level, lines, and session high score. After game over, press `r` to restart or `q`/Escape to return to the menu.
- Clearing 1/2/3/4 lines scores 100/300/500/800 points, multiplied by the current level. Back-to-back Tetrises score 1.5 times the normal Tetris points. Every 10 cleared lines advances a level and speeds up gravity, from one row every 45 ticks at level 1 to every 3 ticks at level 15.
- Pieces lock after 30 ticks (0.5 seconds) on the ground. Successful moves or rotations can reset the lock delay up to 15 times.
- `TetrisGame` (`src/tetris.h` and `src/tetris.cpp`) contains the game rules independently of input and drawing. `playTetris()` handles POSIX raw input and rendering, and restores terminal settings when you leave. A terminal smaller than 80 columns by 24 rows displays a warning and returns to the menu.

Minesweeper (board: 9x9, 10 mines)
- Clear all non-mine cells to win. Revealing a mine loses.
- Commands: `r row col` (reveal), `f row col` (flag/unflag), `q` (quit). Coordinates are 1-9 (1,1 = top-left).
- First reveal is always safe; if it lands on a mine, the mine is moved.
- Revealing a zero-adjacent cell performs flood fill through safe neighbors.

## Code

- `src/main.cpp`: the menu
- `src/game2048.*`, `src/tictactoe.*`, `src/hangman.*`, `src/snake.*`, `src/dino.*`, `src/blackjack.*`, `src/tetris.*`, `src/spaceinvaders.*`: the games
- `tests/tests.cpp`: tests, using a small `CHECK` macro
- `src/intro.*`: Borderland startup animation, scalable trial card rendering, and theme menu
- `src/spaceinvaders.*`: Space Invaders arcade game logic and renderer

## Contributing

Fork the repo, make your changes on a new branch, and open a pull request. Make sure it builds and the tests pass.

If you find a bug, open an issue with the steps to reproduce it, what you expected, and what happened instead.

Part of Source Start by CSI SPIT. MIT licensed.
