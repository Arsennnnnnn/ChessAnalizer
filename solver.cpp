#include "Solver.h"
#include <iostream>
#include <sstream>

// ─── Core recursive search ────────────────────────────────────────────────────
// Returns true if the side to move in `board` is in checkmate after exactly
// `depth` more half-moves, with optimal play from both sides.
//
// solvingTurn = true  → it is the ATTACKER's turn (tries to deliver mate)
// solvingTurn = false → it is the DEFENDER's turn (tries to escape)
//
// The fix vs. the old version: depth counting is now purely ply-based and the
// base case is checked BEFORE consuming a ply, not after. This prevents the
// off-by-one that allowed non-existent "mates" to be reported.
bool Solver::searchMate(const Board& board, int depth, bool solvingTurn,
                         Color sideToSolve, SolveLine& line) const
{
    // ── Base case: no plies left ──────────────────────────────────────────────
    // We only declare success if the DEFENDER (the non-solving side) is
    // in checkmate right now.
    if (depth == 0) {
        Color defender = opposite(sideToSolve);
        return board.isCheckmate(defender);
    }

    Color current = board.turn_;
    auto moves = board.legalMoves(current);

    // No legal moves: checkmate or stalemate
    if (moves.empty()) {
        if (board.isCheckmate(current)) {
            // current player is mated — good only if current is the defender
            return current == opposite(sideToSolve);
        }
        return false; // stalemate = not a win
    }

    if (solvingTurn) {
        // Attacker's turn: need at least ONE move that forces mate
        for (const auto& mv : moves) {
            Board next(board);
            next.applyMove(mv);

            SolveLine subLine;
            if (searchMate(next, depth - 1, false, sideToSolve, subLine)) {
                line.moves.clear();
                line.moves.push_back(mv);
                line.moves.insert(line.moves.end(), subLine.moves.begin(), subLine.moves.end());
                return true;
            }
        }
        return false;
    } else {
        // Defender's turn: ALL moves must lead to forced mate
        SolveLine combinedLine;
        for (const auto& mv : moves) {
            Board next(board);
            next.applyMove(mv);

            SolveLine subLine;
            if (!searchMate(next, depth - 1, true, sideToSolve, subLine)) {
                return false; // defender found an escape
            }
            // Keep the last variation for display purposes
            combinedLine.moves.clear();
            combinedLine.moves.push_back(mv);
            combinedLine.moves.insert(combinedLine.moves.end(), subLine.moves.begin(), subLine.moves.end());
        }
        line = combinedLine;
        return true;
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────
std::vector<SolveLine> Solver::findMateInN(const Board& board, int depth) const {
    std::vector<SolveLine> results;
    Color sideToSolve = board.turn_;
    auto moves = board.legalMoves(sideToSolve);

    for (const auto& mv : moves) {
        Board next(board);
        next.applyMove(mv);

        SolveLine subLine;
        // After attacker's first move, depth-1 plies remain, defender moves next
        if (searchMate(next, depth - 1, false, sideToSolve, subLine)) {
            SolveLine full;
            full.moves.push_back(mv);
            full.moves.insert(full.moves.end(), subLine.moves.begin(), subLine.moves.end());
            results.push_back(full);
        }
    }
    return results;
}

// ─── Algebraic notation helper ───────────────────────────────────────────────
std::string Solver::moveToAlgebraic(const Board& board, const Move& m) {
    const auto& piece = board.cell(m.from);
    if (!piece) return "??";

    std::ostringstream oss;

    if (m.isCastling) {
        oss << (m.to.col > m.from.col ? "O-O" : "O-O-O");
        return oss.str();
    }

    PieceType t = piece->type();
    if (t != PieceType::Pawn)
        oss << (char)toupper(pieceTypeToChar(t)[0]);

    if (t == PieceType::Pawn && m.from.col != m.to.col)
        oss << (char)('a' + m.from.col);

    if (!board.isEmpty(m.to) || m.isEnPassant) oss << 'x';

    oss << (char)('a' + m.to.col);
    oss << (8 - m.to.row);

    if (m.promotion != PieceType::Empty)
        oss << '=' << (char)toupper(pieceTypeToChar(m.promotion)[0]);

    Board next(board);
    next.applyMove(m);
    Color opp = opposite(piece->color());
    if (next.isCheckmate(opp))    oss << '#';
    else if (next.isInCheck(opp)) oss << '+';

    return oss.str();
}

// ─── Print a full line ────────────────────────────────────────────────────────
void Solver::printSolveLine(const Board& boardIn, const SolveLine& line, Color attackerColor) {
    Board b(boardIn);
    int moveNum = 1;
    for (size_t i = 0; i < line.moves.size(); i++) {
        const auto& mv = line.moves[i];
        if (b.turn_ == Color::White) {
            std::cout << moveNum << ". ";
        } else if (i == 0) {
            std::cout << moveNum << "... ";
        }
        std::cout << moveToAlgebraic(b, mv) << "  ";
        if (b.turn_ == Color::Black) moveNum++;
        b.applyMove(mv);
    }
    std::cout << "\n";
}