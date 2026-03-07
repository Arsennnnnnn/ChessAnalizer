# Chess Mate Solver

A C++17 command-line tool that analyzes chess positions and finds forced checkmates in 1, 2, or 3 moves. You input the pieces manually — no game to play, just pure position analysis.

---

## Features

- Detects if a position is **already checkmate or stalemate**
- Finds **mate in 1, 2, or 3** with full move lines
- Shows **all mating first moves** when multiple solutions exist
- Accepts positions via **piece tokens** or **FEN strings**
- **Validates** illegal positions before analysis:
    - Two pieces on the same square
    - More than one king per side
    - Pawns on the 1st or 8th rank
    - Adjacent kings
    - The side that just moved left in check

---

## Build

No external dependencies — only the C++17 standard library.

```bash
g++ -std=c++17 -O2 -I. main.cpp Board.cpp Pieces.cpp Solver.cpp Game.cpp -o solver
```

Or with CMake:

```bash
mkdir build && cd build
cmake .. && make
```

---

## Usage

Run the program and enter pieces at the prompt:

```
./solver
Position> Ke1 Qd1 ke8 rb8
```

### Piece format

Each piece is written as `<Piece><file><rank>`:

| Character | Piece  |
|-----------|--------|
| `K` / `k` | King   |
| `Q` / `q` | Queen  |
| `R` / `r` | Rook   |
| `B` / `b` | Bishop |
| `N` / `n` | Knight |
| `P` / `p` | Pawn   |

**Uppercase = White, lowercase = Black** — same convention as FEN.

Optionally append `w` or `b` to specify whose turn it is. If omitted, **White to move** is assumed.

### Examples

```
# White to move (default)
Position> Ke1 Qd1 ke8 rb8

# Black to move
Position> Ke1 Qd1 ke8 rb8 b

# Load a FEN string
Position> fen r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq -

# Already mated position — detected instantly
Position> Ka6 Rb6 ka8
```

### Commands

| Command | Description |
|---------|-------------|
| `help`  | Show the input guide |
| `quit`  | Exit the program |

---

## Example Session

```
Position> Kf8 Qd5 Nh4 kh8 nh6 pg7 ph7 w

  a b c d e f g h
8 . . . . . K . k 8
7 . . . . . . p p 7
6 . . . . . . . n 6
5 . . . Q . . . . 5
4 . . . . . . . N 4

White to move

Searching for forced mate (up to 3 moves)...

  No mate in 1  (0.16 ms)
>>> Mate in 2!  (1 solution(s), 11.68 ms)

  Best line:  1. Qh1  Ng4  2. Ng6#
```

```
Position> Ka6 Rb6 ka8

>>> This position is already stalemate — it is a draw.
```

```
Position> Kh6 kh8 Ra8 ra8

  Error: square a8 is already occupied — two pieces cannot share a square.
```

---

## File Structure

| File | Description |
|------|-------------|
| `Matrix.h` | Template base class `Matrix<T>` — generic 2D grid with bounds checking |
| `Types.h` | Shared types: `Color`, `PieceType`, `Position`, `Move` |
| `Pieces.h` / `Pieces.cpp` | Abstract `Piece` base class and all 6 piece types with move generation |
| `Board.h` / `Board.cpp` | `Board` class (inherits `Matrix<Cell>`) — applies moves, legal move filtering, check/mate/stalemate detection, FEN import/export |
| `Solver.h` / `Solver.cpp` | Recursive minimax mate search engine |
| `Game.h` / `Game.cpp` | Full game manager with 50-move rule, threefold repetition, insufficient material |
| `main.cpp` | Interactive CLI — input parsing, validation, and solver output |

---

## How It Works

### Architecture

`Board` inherits from the template class `Matrix<shared_ptr<Piece>>`. Each of the 6 piece types inherits from an abstract `Piece` base class and implements `pseudoLegalMoves()` and `clone()`.

### Move Generation

Move generation works in two stages:

1. **Pseudo-legal** — each piece generates all moves it can make ignoring whether the king ends up in check
2. **Legal filtering** — each pseudo-legal move is applied to a board copy; if the moving side's king is in check afterward, the move is discarded

Special moves fully supported: castling, en passant, and pawn promotion.

### Mate Search

The solver uses a **recursive minimax** search:

- **Attacker's turn** — needs at least one move that forces mate regardless of what the defender does
- **Defender's turn** — ALL legal responses must still lead to forced mate; one escape refutes the line

Depth is counted in **plies** (half-moves): mate in N requires `2N - 1` plies. The search is brute-force without alpha-beta pruning, which is sufficient for depth 3 (under 15 ms on typical positions).

---

## Limitations

- Solver searches up to **mate in 3** only
- No engine play — analysis only
- Algebraic notation output is simplified (no full disambiguation for pieces of the same type)