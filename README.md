# terminal-games

2048, tic-tac-toe against the computer, hangman, and snake in the terminal. Written in C++17.

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
- Anything other than 1 to 5, including letters, prints an error and asks again.

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

Snake (arrow keys or `w a s d` to steer, `q` to quit)
- Control the snake to eat food (`*`) and grow. Each food eaten adds 10 to the score.
- Steer with the arrow keys or `w` (up), `a` (left), `s` (down), `d` (right).
- The snake wraps around all boundaries (passing through one side brings you out the opposite side).
- The game ends if the snake runs into its own body.

## Code

- `src/main.cpp`: the menu
- `src/game2048.*`, `src/tictactoe.*`, `src/hangman.*`, `src/snake.*`: the games
- `tests/tests.cpp`: tests, using a small `CHECK` macro

## Contributing

Fork the repo, make your changes on a new branch, and open a pull request. Make sure it builds and the tests pass.

If you find a bug, open an issue with the steps to reproduce it, what you expected, and what happened instead.

Part of Source Start by CSI SPIT. MIT licensed.
