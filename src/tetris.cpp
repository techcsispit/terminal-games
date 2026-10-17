#include "tetris.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <utility>

namespace {
using Cells = std::array<TetrisPoint, 4>;
using Kicks = std::array<TetrisPoint, 5>;
constexpr int INITIAL_GRAVITY_TICKS = 45;
constexpr int FASTEST_GRAVITY_TICKS = 3;

constexpr std::array<Cells, 4> I_CELLS{{
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
    {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    {{{0, 2}, {1, 2}, {2, 2}, {3, 2}}},
    {{{1, 0}, {1, 1}, {1, 2}, {1, 3}}}
}};
constexpr std::array<Cells, 4> O_CELLS{{
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}}
}};
constexpr std::array<Cells, 4> T_CELLS{{
    {{{0, 1}, {1, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {2, 1}}},
    {{{0, 1}, {1, 1}, {1, 2}, {2, 1}}},
    {{{0, 1}, {1, 0}, {1, 1}, {1, 2}}}
}};
constexpr std::array<Cells, 4> S_CELLS{{
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    {{{1, 1}, {2, 1}, {0, 2}, {1, 2}}},
    {{{0, 0}, {0, 1}, {1, 1}, {1, 2}}}
}};
constexpr std::array<Cells, 4> Z_CELLS{{
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    {{{2, 0}, {1, 1}, {2, 1}, {1, 2}}},
    {{{0, 1}, {1, 1}, {1, 2}, {2, 2}}},
    {{{1, 0}, {0, 1}, {1, 1}, {0, 2}}}
}};
constexpr std::array<Cells, 4> J_CELLS{{
    {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {1, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {2, 2}}},
    {{{1, 0}, {1, 1}, {0, 2}, {1, 2}}}
}};
constexpr std::array<Cells, 4> L_CELLS{{
    {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {0, 2}}},
    {{{0, 0}, {1, 0}, {1, 1}, {1, 2}}}
}};

const Cells& cellsFor(Tetromino piece, int rotation) {
    const int state = rotation & 3;
    switch (piece) {
        case Tetromino::I: return I_CELLS[state];
        case Tetromino::O: return O_CELLS[state];
        case Tetromino::T: return T_CELLS[state];
        case Tetromino::S: return S_CELLS[state];
        case Tetromino::Z: return Z_CELLS[state];
        case Tetromino::J: return J_CELLS[state];
        case Tetromino::L: return L_CELLS[state];
    }
    return I_CELLS[0];
}

constexpr Kicks ZERO_KICK{{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}};

Kicks normalKicks(int from, int to) {
    if (from == 0 && to == 1) return {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}};
    if (from == 1 && to == 0) return {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}};
    if (from == 1 && to == 2) return {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}};
    if (from == 2 && to == 1) return {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}};
    if (from == 2 && to == 3) return {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}};
    if (from == 3 && to == 2) return {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}};
    if (from == 3 && to == 0) return {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}};
    if (from == 0 && to == 3) return {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}};
    return ZERO_KICK;
}

Kicks iKicks(int from, int to) {
    if (from == 0 && to == 1) return {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}};
    if (from == 1 && to == 0) return {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}};
    if (from == 1 && to == 2) return {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}};
    if (from == 2 && to == 1) return {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}};
    if (from == 2 && to == 3) return {{{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}};
    if (from == 3 && to == 2) return {{{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}};
    if (from == 3 && to == 0) return {{{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}};
    if (from == 0 && to == 3) return {{{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}};
    return ZERO_KICK;
}
}  // namespace

TetrisGame::TetrisGame(std::uint32_t seed) : rng_(seed) {
    queue_.reserve(4);
    currentPiece_ = drawPiece();
    refillQueue();
}

void TetrisGame::refillQueue() {
    while (queue_.size() < 3) queue_.push_back(drawPiece());
}

Tetromino TetrisGame::drawPiece() {
    if (bagIndex_ >= bag_.size()) {
        bag_ = {Tetromino::I, Tetromino::O, Tetromino::T, Tetromino::S,
                Tetromino::Z, Tetromino::J, Tetromino::L};
        std::shuffle(bag_.begin(), bag_.end(), rng_);
        bagIndex_ = 0;
    }
    return bag_[bagIndex_++];
}

void TetrisGame::placeAtSpawn(Tetromino piece) {
    currentPiece_ = piece;
    rotation_ = 0;
    x_ = 3;
    y_ = 0;
    gravityTicks_ = 0;
    lockTicks_ = 0;
    lockResets_ = 0;
    if (!canPlace(currentPiece_, rotation_, x_, y_)) gameOver_ = true;
}

void TetrisGame::spawnNext() {
    if (queue_.empty()) queue_.push_back(drawPiece());
    const Tetromino next = queue_.front();
    queue_.erase(queue_.begin());
    queue_.push_back(drawPiece());
    holdUsed_ = false;
    placeAtSpawn(next);
}

std::array<Tetromino, 3> TetrisGame::nextPieces() const {
    return {{queue_[0], queue_[1], queue_[2]}};
}

bool TetrisGame::canPlace(Tetromino piece, int rotation, int x, int y) const {
    for (const TetrisPoint cell : cellsFor(piece, rotation)) {
        const int boardX = x + cell.x;
        const int boardY = y + cell.y;
        if (boardX < 0 || boardX >= WIDTH || boardY < 0 || boardY >= HEIGHT ||
            board_[boardY][boardX] != 0) {
            return false;
        }
    }
    return true;
}

bool TetrisGame::isGrounded() const {
    return !canPlace(currentPiece_, rotation_, x_, y_ + 1);
}

void TetrisGame::resetLockDelayIfNeeded(bool wasGrounded) {
    if ((wasGrounded || isGrounded()) && lockResets_ < MAX_LOCK_RESETS) {
        lockTicks_ = 0;
        ++lockResets_;
    }
}

bool TetrisGame::move(int dx, int dy) {
    if (gameOver_ || !canPlace(currentPiece_, rotation_, x_ + dx, y_ + dy)) return false;
    const bool wasGrounded = isGrounded();
    x_ += dx;
    y_ += dy;
    if (dy != 0) gravityTicks_ = 0;
    if (dy == 0) resetLockDelayIfNeeded(wasGrounded);
    else lockTicks_ = 0;
    return true;
}

bool TetrisGame::moveLeft() { return move(-1, 0); }
bool TetrisGame::moveRight() { return move(1, 0); }

bool TetrisGame::softDrop() {
    if (!move(0, 1)) return false;
    ++score_;
    return true;
}

int TetrisGame::ghostY() const {
    int result = y_;
    while (canPlace(currentPiece_, rotation_, x_, result + 1)) ++result;
    return result;
}

int TetrisGame::hardDrop() {
    if (gameOver_) return 0;
    const int distance = ghostY() - y_;
    y_ += distance;
    score_ += distance * 2;
    lockPiece();
    return distance;
}

bool TetrisGame::rotate(int direction) {
    if (gameOver_ || currentPiece_ == Tetromino::O) return false;
    const int target = (rotation_ + direction + 4) % 4;
    const bool wasGrounded = isGrounded();
    const Kicks kicks = currentPiece_ == Tetromino::I
        ? iKicks(rotation_, target) : normalKicks(rotation_, target);
    for (const TetrisPoint kick : kicks) {
        if (canPlace(currentPiece_, target, x_ + kick.x, y_ + kick.y)) {
            rotation_ = target;
            x_ += kick.x;
            y_ += kick.y;
            resetLockDelayIfNeeded(wasGrounded);
            return true;
        }
    }
    return false;
}

bool TetrisGame::rotateClockwise() { return rotate(1); }
bool TetrisGame::rotateCounterClockwise() { return rotate(-1); }

bool TetrisGame::hold() {
    if (!canHold()) return false;
    const Tetromino outgoing = currentPiece_;
    if (heldPieceValid_) {
        placeAtSpawn(heldPiece_);
    } else {
        heldPiece_ = outgoing;
        heldPieceValid_ = true;
        if (queue_.empty()) queue_.push_back(drawPiece());
        const Tetromino next = queue_.front();
        queue_.erase(queue_.begin());
        queue_.push_back(drawPiece());
        placeAtSpawn(next);
    }
    holdUsed_ = true;
    return true;
}

void TetrisGame::tick() {
    if (gameOver_) return;
    if (isGrounded()) {
        ++lockTicks_;
        if (lockTicks_ >= LOCK_DELAY_TICKS) lockPiece();
        return;
    }
    ++gravityTicks_;
    if (gravityTicks_ >= gravityPeriod()) {
        gravityTicks_ = 0;
        if (!move(0, 1)) {
            lockTicks_ = 0;
        }
    }
}

int TetrisGame::gravityPeriod() const {
    const double levelProgress = static_cast<double>(level() - 1) / 14.0;
    const double ratio = static_cast<double>(FASTEST_GRAVITY_TICKS) /
                         INITIAL_GRAVITY_TICKS;
    return std::max(FASTEST_GRAVITY_TICKS, static_cast<int>(std::lround(
        INITIAL_GRAVITY_TICKS * std::pow(ratio, levelProgress))));
}

void TetrisGame::lockPiece() {
    for (const TetrisPoint cell : cellsFor(currentPiece_, rotation_)) {
        const int boardX = x_ + cell.x;
        const int boardY = y_ + cell.y;
        if (boardY >= 0 && boardY < HEIGHT && boardX >= 0 && boardX < WIDTH) {
            board_[boardY][boardX] = static_cast<int>(currentPiece_) + 1;
        }
    }
    clearLines();
    spawnNext();
}

void TetrisGame::clearLines() {
    int cleared = 0;
    for (int row = HEIGHT - 1; row >= 0;) {
        if (std::all_of(board_[row].begin(), board_[row].end(),
                        [](int cell) { return cell != 0; })) {
            for (int shift = row; shift > 0; --shift) board_[shift] = board_[shift - 1];
            board_[0].fill(0);
            ++cleared;
        } else {
            --row;
        }
    }

    if (cleared == 0) return;
    const int currentLevel = level();
    static constexpr int points[] = {0, 100, 300, 500, 800};
    if (cleared == 4 && backToBackTetris_) {
        score_ += 1200 * currentLevel;
    } else {
        score_ += points[cleared] * currentLevel;
    }
    backToBackTetris_ = cleared == 4;
    lines_ += cleared;
}

void TetrisGame::setBoardForTesting(const Board& board) {
    board_ = board;
}

void TetrisGame::setCurrentForTesting(Tetromino piece, int rotation, int x, int y) {
    currentPiece_ = piece;
    rotation_ = rotation & 3;
    x_ = x;
    y_ = y;
    gameOver_ = !canPlace(currentPiece_, rotation_, x_, y_);
    gravityTicks_ = 0;
    lockTicks_ = 0;
    lockResets_ = 0;
}

void TetrisGame::setLinesForTesting(int lines) {
    lines_ = std::max(0, lines);
}

bool TetrisGame::isValidState() const {
    if (score_ < 0 || lines_ < 0 || bagIndex_ > bag_.size() || queue_.size() < 3) return false;
    for (const auto& row : board_) {
        for (int cell : row) {
            if (cell < 0 || cell > 7) return false;
        }
    }
    return gameOver_ || canPlace(currentPiece_, rotation_, x_, y_);
}

namespace {
constexpr int BOARD_LEFT_WIDTH = 28;
constexpr int FRAME_ROWS = TetrisGame::VISIBLE_HEIGHT + 2;
constexpr int FRAME_TICK_MICROSECONDS = 1000000 / 60;
constexpr int ESCAPE_SEQUENCE_TICKS = 6;
constexpr int PIECE_COLORS[] = {51, 226, 165, 46, 196, 27, 208};
constexpr int GHOST_COLORS[] = {239, 242, 239, 238, 239, 240, 238};

int pieceColor(Tetromino piece, bool ghost = false) {
    const int index = static_cast<int>(piece);
    return ghost ? GHOST_COLORS[index] : PIECE_COLORS[index];
}

std::string coloredCells(Tetromino piece, const char* cells, bool ghost = false) {
    return "\033[38;5;" + std::to_string(pieceColor(piece, ghost)) + "m" +
           cells + "\033[0m";
}

std::string miniPieceLine(Tetromino piece, int row) {
    std::string line;
    const auto& cells = cellsFor(piece, 0);
    for (int column = 0; column < 4; ++column) {
        bool occupied = false;
        for (const TetrisPoint cell : cells) {
            if (cell.x == column && cell.y == row) {
                occupied = true;
                break;
            }
        }
        line += occupied ? coloredCells(piece, "[]") : "  ";
    }
    return line;
}

std::string sectionTitle(const std::string& title) {
    return "\033[1;38;5;45m" + title + "\033[0m";
}

std::string muted(const std::string& text) {
    return "\033[38;5;244m" + text + "\033[0m";
}

std::string statLine(const std::string& label, const std::string& value,
                     bool highlight = false) {
    const std::string labelColor = "\033[38;5;244m";
    const std::string valueColor = highlight
        ? "\033[1;38;5;226m" : "\033[1;38;5;255m";
    std::string paddedLabel = label;
    if (paddedLabel.size() < 7) paddedLabel.append(7 - paddedLabel.size(), ' ');
    return labelColor + paddedLabel + "\033[0m" + valueColor + value + "\033[0m";
}

std::string controlLine(const std::string& label, const std::string& keys) {
    std::string paddedLabel = label;
    if (paddedLabel.size() < 12) paddedLabel.append(12 - paddedLabel.size(), ' ');
    return muted(paddedLabel) + "\033[1;38;5;255m" + keys + "\033[0m";
}

std::size_t visibleLength(const std::string& text) {
    std::size_t visible = 0;
    for (std::size_t i = 0; i < text.size();) {
        if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '[') {
            i += 2;
            while (i < text.size() && text[i] != 'm') ++i;
            if (i < text.size()) ++i;
        } else {
            ++visible;
            ++i;
        }
    }
    return visible;
}

void padTo(std::string& text, std::size_t width) {
    const std::size_t length = visibleLength(text);
    if (length < width) text.append(width - length, ' ');
}

std::string makeFrame(const TetrisGame& game, int highScore, bool paused) {
    std::array<std::string, FRAME_ROWS> left{};
    std::array<std::string, FRAME_ROWS> right{};
    left[0] = sectionTitle("HOLD");
    if (game.hasHeldPiece()) {
        for (int row = 0; row < 4; ++row) left[row + 1] = miniPieceLine(game.heldPiece(), row);
    } else {
        left[2] = muted("no piece held");
    }
    left[5] = muted("+--------------------------+");
    left[6] = sectionTitle("RUN STATS");
    left[7] = statLine("SCORE", std::to_string(game.score()), true);
    left[8] = statLine("LEVEL", std::to_string(game.level()));
    left[9] = statLine("LINES", std::to_string(game.lines()) + " / 10");
    left[10] = statLine("BEST", std::to_string(highScore));
    const int levelProgress = game.lines() % 10;
    left[11] = muted("PROGRESS ");
    for (int unit = 0; unit < 10; ++unit) {
        left[11] += unit < levelProgress
            ? "\033[38;5;46m#\033[0m" : muted(".");
    }
    left[12] = sectionTitle("CONTROLS");
    left[13] = controlLine("MOVE", "A D / <- ->");
    left[14] = controlLine("SOFT DROP", "S / DOWN");
    left[15] = controlLine("ROTATE CW", "X / UP");
    left[16] = controlLine("ROTATE CCW", "Z");
    left[17] = controlLine("HARD DROP", "SPACE");
    left[18] = controlLine("HOLD", "C");
    left[19] = controlLine("PAUSE", "P");
    left[20] = controlLine("QUIT", "Q / ESC");

    right[0] = sectionTitle("NEXT PIECES");
    const auto next = game.nextPieces();
    for (int piece = 0; piece < 3; ++piece) {
        const int start = 1 + piece * 6;
        for (int row = 0; row < 4; ++row) right[start + row] = miniPieceLine(next[piece], row);
    }
    right[19] = "Arrows or a/d/s";
    right[20] = "x/up CW, z CCW";
    if (paused) {
        right[10] = "\033[1;38;5;226m== PAUSED ==\033[0m";
        right[11] = "press P to resume";
    } else if (game.isGameOver()) {
        right[10] = "\033[1;38;5;196m== GAME OVER ==\033[0m";
        right[11] = statLine("SCORE", std::to_string(game.score()), true);
        right[12] = statLine("LEVEL", std::to_string(game.level()));
        right[13] = statLine("LINES", std::to_string(game.lines()));
        right[15] = controlLine("PLAY AGAIN", "R");
        right[16] = controlLine("MENU", "Q / ESC");
    }

    std::string frame = "\033[H";
    for (int line = 0; line < FRAME_ROWS; ++line) {
        std::string leftText = left[line];
        padTo(leftText, BOARD_LEFT_WIDTH);
        frame += leftText;
        if (line == 0) {
            frame += '+' + std::string(TetrisGame::WIDTH * 2, '-') + '+';
        } else if (line == FRAME_ROWS - 1) {
            frame += '+' + std::string(TetrisGame::WIDTH * 2, '-') + '+';
        } else {
            const int boardY = line - 1 + TetrisGame::HIDDEN_ROWS;
            frame += '|';
            const int ghost = game.ghostY();
            for (int x = 0; x < TetrisGame::WIDTH; ++x) {
                int value = game.board()[boardY][x];
                bool ghostCell = false;
                bool activeCell = false;
                for (const TetrisPoint cell : cellsFor(game.currentPiece(), game.rotation())) {
                    const int pieceX = game.x() + cell.x;
                    const int pieceY = game.y() + cell.y;
                    if (pieceX == x && pieceY == boardY) activeCell = true;
                    if (pieceX == x && pieceY + (ghost - game.y()) == boardY) ghostCell = true;
                }
                if (activeCell) {
                    frame += coloredCells(game.currentPiece(), "[]");
                } else if (value != 0) {
                    frame += coloredCells(static_cast<Tetromino>(value - 1), "[]");
                } else if (ghostCell && !game.isGameOver()) {
                    frame += coloredCells(game.currentPiece(), "::", true);
                } else {
                    frame += "  ";
                }
            }
            frame += '|';
        }
        frame += "  " + right[line] + "\033[K\n";
    }
    if (game.isGameOver()) {
        frame += "\r\nGAME OVER  Score: " + std::to_string(game.score()) +
                 "  Level: " + std::to_string(game.level()) +
                 "  Lines: " + std::to_string(game.lines()) +
                 "  Session high score: " + std::to_string(highScore) +
                 "\r\nPress r to restart or q to return to the menu.\033[K";
    }
    return frame;
}

class TetrisTerminal {
public:
    bool start() {
        if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
            std::cerr << "Tetris needs an interactive terminal.\n";
            return false;
        }
        if (tcgetattr(STDIN_FILENO, &original_) != 0) {
            std::cerr << "Unable to read terminal settings.\n";
            return false;
        }
        termiosReady_ = true;
        termios raw = original_;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO | ISIG));
        raw.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
            std::cerr << "Unable to enable raw terminal input.\n";
            return false;
        }
        active_ = true;
        const char* enterScreen = "\033[?1049h\033[?25l\033[?7l\033[2J\033[H";
        const std::size_t length = std::char_traits<char>::length(enterScreen);
        if (::write(STDOUT_FILENO, enterScreen, length) != static_cast<ssize_t>(length)) {
            std::cerr << "Unable to initialize terminal display.\n";
            return false;
        }
        return true;
    }

    ~TetrisTerminal() {
        if (active_) {
            const char* leaveScreen = "\033[?7h\033[?25h\033[?1049l";
            const std::size_t length = std::char_traits<char>::length(leaveScreen);
            if (::write(STDOUT_FILENO, leaveScreen, length) != static_cast<ssize_t>(length)) {
                std::cerr << "Unable to restore terminal display.\n";
            }
        }
        if (termiosReady_ && tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_) != 0) {
            std::cerr << "Unable to restore terminal settings.\n";
        }
    }

private:
    termios original_{};
    bool termiosReady_ = false;
    bool active_ = false;
};

bool terminalIsLargeEnough() {
    winsize size{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0) {
        std::cerr << "Unable to determine terminal size for Tetris.\n";
        return false;
    }
    if (size.ws_col < 80 || size.ws_row < 24) {
        std::cerr << "Terminal is too small for Tetris; use at least 80 columns by 24 rows.\n";
        return false;
    }
    return true;
}

}  // namespace

void playTetris() {
    if (!terminalIsLargeEnough()) return;
    TetrisTerminal terminal;
    if (!terminal.start()) return;

    static int sessionHighScore = 0;
    TetrisGame game;
    bool paused = false;
    bool quit = false;
    int escapeState = 0;
    int escapeTicks = 0;
    auto nextTick = std::chrono::steady_clock::now() +
                    std::chrono::microseconds(FRAME_TICK_MICROSECONDS);

    auto handleKey = [&](unsigned char key) {
        if (key == 3 || key == 27) {
            quit = true;
            return;
        }
        const char ch = static_cast<char>(std::tolower(key));
        if (paused && ch != 'p' && ch != 'q') return;
        if (game.isGameOver()) {
            if (ch == 'r') {
                game = TetrisGame();
                paused = false;
            } else if (ch == 'q') {
                quit = true;
            }
            return;
        }
        switch (ch) {
            case 'a': game.moveLeft(); break;
            case 'd': game.moveRight(); break;
            case 's': game.softDrop(); break;
            case 'x': game.rotateClockwise(); break;
            case 'z': game.rotateCounterClockwise(); break;
            case ' ': game.hardDrop(); break;
            case 'c': game.hold(); break;
            case 'p': paused = !paused; break;
            case 'q': quit = true; break;
            default: break;
        }
    };

    while (!quit) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);
        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
            nextTick - now);
        timeval timeout{};
        const auto waitMicros = std::max<std::int64_t>(0, remaining.count());
        timeout.tv_sec = static_cast<time_t>(waitMicros / 1000000);
        timeout.tv_usec = static_cast<suseconds_t>(waitMicros % 1000000);
        const int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Tetris input error.\n";
            return;
        }
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &readSet)) {
            unsigned char input[64];
            const ssize_t count = read(STDIN_FILENO, input, sizeof(input));
            if (count < 0 && errno != EINTR) {
                std::cerr << "Unable to read Tetris input.\n";
                return;
            }
            for (ssize_t i = 0; i < count && !quit; ++i) {
                const unsigned char byte = input[i];
                if (escapeState == 1) {
                    if (byte == '[') {
                        escapeState = 2;
                    } else {
                        quit = true;
                    }
                } else if (escapeState == 2) {
                    switch (byte) {
                        case 'A': if (!paused) game.rotateClockwise(); break;
                        case 'B': if (!paused) game.softDrop(); break;
                        case 'C': if (!paused) game.moveRight(); break;
                        case 'D': if (!paused) game.moveLeft(); break;
                        default: break;
                    }
                    escapeState = 0;
                } else if (byte == 27) {
                    escapeState = 1;
                    escapeTicks = 0;
                } else {
                    handleKey(byte);
                }
            }
        }

        const auto afterInput = std::chrono::steady_clock::now();
        while (afterInput >= nextTick) {
            nextTick += std::chrono::microseconds(FRAME_TICK_MICROSECONDS);
            if (escapeState == 1 && ++escapeTicks >= ESCAPE_SEQUENCE_TICKS) quit = true;
            if (!paused && !game.isGameOver()) game.tick();
            sessionHighScore = std::max(sessionHighScore, game.score());
        }

        if (!quit) {
            const std::string frame = makeFrame(game, sessionHighScore, paused);
            const ssize_t written = ::write(STDOUT_FILENO, frame.data(), frame.size());
            if (written < 0 || static_cast<std::size_t>(written) != frame.size()) {
                std::cerr << "Unable to draw Tetris frame.\n";
                return;
            }
        }
    }
}
