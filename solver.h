#ifndef SOLVER_H
#define SOLVER_H
#include "Board.h"
#include <vector>
#include <optional>
#include <string>

// Represents a solution line (sequence of moves)
struct SolveLine {
    std::vector<Move> moves;
};

class Solver {
public:
    Solver() = default;

    // Returns the list of moves that deliver mate in exactly 'depth' moves.
    // depth=1: mate in 1, depth=2: mate in 2, depth=3: mate in 3
    // Returns empty vector if no such mate exists.
    std::vector<SolveLine> findMateInN(const Board& board, int depth) const;

    // Convenience wrappers
    std::vector<SolveLine> findMateInOne  (const Board& board) const { return findMateInN(board, 1); }
    std::vector<SolveLine> findMateInTwo  (const Board& board) const { return findMateInN(board, 2); }
    std::vector<SolveLine> findMateInThree(const Board& board) const { return findMateInN(board, 3); }

    // Pretty-print a move in algebraic notation
    static std::string moveToAlgebraic(const Board& board, const Move& m);

    // Print full solve line
    static void printSolveLine(const Board& board, const SolveLine& line, Color attackerColor);

private:
    // Returns true if the position (with `color` to move) is mated in exactly `depth` half-moves
    // sideToSolve: the color trying to deliver mate
    // The `line` is filled with the principal variation
    bool searchMate(const Board& board, int depth, bool solvingTurn, Color sideToSolve,
                    SolveLine& line) const;
};

#endif // SOLVER_H