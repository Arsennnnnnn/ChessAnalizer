# Chess Mate Solver

A chess position analyzer that determines whether a given position contains a forced checkmate. Given a set of pieces on the board, it will tell you if there is mate in 1, 2, or 3 moves and show you the exact move line to get there.

---

## Build

```bash
g++ -std=c++17 -O2 -I. main.cpp Board.cpp Pieces.cpp Solver.cpp Game.cpp -o solver
```

---

## Usage

```
./solver
Position> Ke1 Qd1 ke8 rb8
```

Each piece is written as `<Piece><file><rank>`. **Uppercase = White, lowercase = Black.**

| Letter | Piece  |
|--------|--------|
| K / k  | King   |
| Q / q  | Queen  |
| R / r  | Rook   |
| B / b  | Bishop |
| N / n  | Knight |
| P / p  | Pawn   |

Append `w` or `b` to specify whose turn it is. If omitted, **White to move** is assumed.

You can also load a FEN string directly:
```
Position> fen r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq -
```

---

## Example Output

```
Position> Kf8 Qd5 Nh4 kh8 nh6 pg7 ph7

  a b c d e f g h
8 . . . . . K . k 8
7 . . . . . . p p 7
6 . . . . . . . n 6
5 . . . Q . . . . 5
4 . . . . . . . N 4
3 . . . . . . . . 3
2 . . . . . . . . 2
1 . . . . . . . . 1
  a b c d e f g h

White to move

  No mate in 1  (0.16 ms)
>>> Mate in 2!  (1 solution(s), 11.68 ms)

  Best line:  1. Qh1  Ng4  2. Ng6#
```

---

## Input Validation

The program rejects illegal positions before analysis:

- Two pieces on the same square
- More than one king per side
- Pawn on the 1st or 8th rank
- The two kings adjacent to each other
- The side that just moved left in check *(checkmate is allowed — it is a valid final position)*

---

## Files

### `Matrix.h`
A generic template class `Matrix<T>` that provides a 2D grid with bounds-checked access via `at(row, col)`. `Board` inherits from it.

### `Types.h`
All shared enums and structs: `Color` (White/Black), `PieceType`, `Position` (row + col), and `Move` (from, to, promotion flag, castling flag, en passant flag). No dependencies on other project files.

### `Pieces.h` / `Pieces.cpp`
Abstract base class `Piece` with fields for color, type, and a `hasMoved` flag. Each of the 6 piece types is a derived class that implements:
- `pseudoLegalMoves()` — generates all moves the piece can make, ignoring whether the king ends up in check
- `clone()` — returns a deep copy of the piece, used when the solver copies the board

Move generation details per piece:
- **Pawn** — single push, double push from start rank, diagonal captures, en passant, promotion (generates 4 separate moves per promotion square: Q, R, B, N)
- **Knight** — 8 hardcoded offset pairs, no sliding
- **Bishop / Rook / Queen** — sliding rays via a shared `addSlidingMoves()` helper that steps in a direction until it hits the board edge or another piece
- **King** — 8 adjacent squares plus castling (king-side and queen-side), with a guard to avoid infinite recursion (see Board section)

A `makePiece(type, color)` factory function creates any piece by type.

### `Board.h` / `Board.cpp`
`Board` inherits `Matrix<shared_ptr<Piece>>`. An empty square is a null pointer. Beyond the 8x8 grid it stores whose turn it is, the en passant target square, and castling rights for both sides.

Key responsibilities:
- **`applyMove()`** — handles the 4 special cases: normal move, castling (moves both king and rook), en passant (removes the captured pawn from a different row than the destination), and promotion (replaces the pawn with the chosen piece)
- **`legalMoves()`** — calls `pseudoLegalMovesAll()` then filters by copying the board, applying each move, and checking if the moving side's king is still in check
- **`isAttackedBy()`** — checks whether a square is attacked by a given color. The King is handled directly here (checking its 8 neighbours with arithmetic) to avoid infinite recursion: King's `pseudoLegalMoves` calls `isInCheck` which calls `isAttackedBy` which would call King's `pseudoLegalMoves` again
- **`isCheckmate()` / `isStalemate()`** — check + no legal moves, or no check + no legal moves
- **`fromFEN()` / `toFEN()`** — standard chess FEN string import and export

### `Solver.h` / `Solver.cpp`
The core search engine. Uses a **recursive minimax** approach:

- **Attacker's turn** — iterates all legal moves. Returns true as soon as one move leads to forced mate. Fills the `SolveLine` with that move and the continuation.
- **Defender's turn** — iterates all legal moves. Returns false as soon as one move escapes. Only returns true if *every* defender response still leads to mate.

Depth is counted in **plies** (half-moves). Mate in N moves = `2N - 1` plies. The public `findMateInN(board, N)` handles this conversion internally. It returns all mating first moves, not just one.

`moveToAlgebraic()` converts a `Move` to standard notation (piece letter, captures, destination, check/mate symbol). `printSolveLine()` prints a full variation with move numbers.

### `Game.h` / `Game.cpp`
A game manager built on top of `Board`. Tracks move history, enforces the 50-move rule (100 half-moves without a pawn move or capture), detects threefold repetition by comparing FEN strings, and checks for insufficient material (K vs K, K+minor vs K). Not used by the solver UI but kept as a complete component.

### `main.cpp`
The interactive CLI. Parses piece tokens, runs all validation checks, and calls `solvePosition()`. Before starting the search, it checks for existing checkmate or stalemate on both sides, so positions that are already over are reported immediately.

---

## Limitations

- Searches up to **mate in 3** only
- No engine play — analysis only
- Algebraic notation is simplified (no disambiguation when two identical pieces can reach the same square)