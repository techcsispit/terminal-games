# terminal-games

2048, tic-tac-toe against the computer, and hangman in the terminal. Written in C++17.

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
- Anything other than 1 to 4, including letters, prints an error and asks again.

2048 (`w a s d` to move, `u` to undo)
- Tiles slide as far as they can. Equal tiles that meet merge, and a tile only merges once per move: `2 2 2 2` moved left is `4 4 _ _`.
- The score goes up by the value of each merged tile.
- A new tile appears after a move only if something actually moved.
- Undo restores the board, score, and random state from before the most recent successful move.
- The game ends when no move is possible.

Tic-tac-toe
- Three in a row wins: rows, columns, and both diagonals.
- The computer wins if it can, otherwise blocks you, otherwise takes the centre; if a corner would set up a fork, it chooses an edge instead.

Hangman
- Upper and lower case letters count as the same guess.
- A wrong guess costs a life, repeating a guess costs nothing. You get 6 lives.

## Code

- `src/main.cpp`: the menu
- `src/game2048.*`, `src/tictactoe.*`, `src/hangman.*`: the games
- `tests/tests.cpp`: tests, using a small `CHECK` macro

## Contributing

Fork the repo, make your changes on a new branch, and open a pull request. Make sure it builds and the tests pass.

If you find a bug, open an issue with the steps to reproduce it, what you expected, and what happened instead.

Part of Source Start by CSI SPIT. MIT licensed.
