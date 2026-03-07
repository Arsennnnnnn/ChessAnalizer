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

    std::vector<SolveLine> findMateInN(const Board& board, int depth) const;

    std::vector<SolveLine> findMateInOne  (const Board& board) const { return findMateInN(board, 1); }
    std::vector<SolveLine> findMateInTwo  (const Board& board) const { return findMateInN(board, 2); }
    std::vector<SolveLine> findMateInThree(const Board& board) const { return findMateInN(board, 3); }

    static std::string moveToAlgebraic(const Board& board, const Move& m);

    static void printSolveLine(const Board& board, const SolveLine& line, Color attackerColor);

private:
    bool searchMate(const Board& board, int depth, bool solvingTurn, Color sideToSolve,
                    SolveLine& line) const;
};

#endif // SOLVER_H