#include "Pieces.h"
#include "Board.h"
#include <algorithm>

// ─── Helper: sliding moves ────────────────────────────────────────────────────
static void addSlidingMoves(
    const Board& board, Position pos, Color color,
    const std::vector<std::pair<int,int>>& dirs,
    std::vector<Move>& moves)
{
    for (auto [dr, dc] : dirs) {
        for (int step = 1; step < 8; step++) {
            Position to{pos.row + dr*step, pos.col + dc*step};
            if (!to.isValid()) break;
            if (board.isEmpty(to)) {
                moves.push_back({pos, to});
            } else {
                if (board.cell(to)->color() != color)
                    moves.push_back({pos, to}); // capture
                break;
            }
        }
    }
}

// ─── Pawn ────────────────────────────────────────────────────────────────────
std::vector<Move> Pawn::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;
    int dir   = (color_ == Color::White) ? -1 : 1;
    int start = (color_ == Color::White) ?  6  : 1;
    int promo = (color_ == Color::White) ?  0  : 7;

    auto addPawnMove = [&](Position from, Position to) {
        if (to.row == promo) {
            for (auto t : {PieceType::Queen, PieceType::Rook,
                           PieceType::Bishop, PieceType::Knight})
                moves.push_back({from, to, t});
        } else {
            moves.push_back({from, to});
        }
    };

    // Single push
    Position fwd{pos.row + dir, pos.col};
    if (fwd.isValid() && board.isEmpty(fwd)) {
        addPawnMove(pos, fwd);
        // Double push
        if (pos.row == start) {
            Position fwd2{pos.row + 2*dir, pos.col};
            if (fwd2.isValid() && board.isEmpty(fwd2))
                moves.push_back({pos, fwd2});
        }
    }

    // Captures
    for (int dc : {-1, 1}) {
        Position cap{pos.row + dir, pos.col + dc};
        if (!cap.isValid()) continue;

        // Normal capture
        if (!board.isEmpty(cap) && board.cell(cap)->color() != color_)
            addPawnMove(pos, cap);

        // En passant
        if (cap == board.enPassantTarget_)
            moves.push_back({pos, cap, PieceType::Empty, false, true});
    }

    return moves;
}

std::unique_ptr<Piece> Pawn::clone() const {
    auto p = std::make_unique<Pawn>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}

// ─── Knight ──────────────────────────────────────────────────────────────────
std::vector<Move> Knight::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;
    static const int kdr[] = {-2,-2,-1,-1,1,1,2,2};
    static const int kdc[] = {-1, 1,-2, 2,-2,2,-1,1};
    for (int i = 0; i < 8; i++) {
        Position to{pos.row+kdr[i], pos.col+kdc[i]};
        if (!to.isValid()) continue;
        if (board.isEmpty(to) || board.cell(to)->color() != color_)
            moves.push_back({pos, to});
    }
    return moves;
}

std::unique_ptr<Piece> Knight::clone() const {
    auto p = std::make_unique<Knight>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}

// ─── Bishop ──────────────────────────────────────────────────────────────────
std::vector<Move> Bishop::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;
    addSlidingMoves(board, pos, color_, {{-1,-1},{-1,1},{1,-1},{1,1}}, moves);
    return moves;
}

std::unique_ptr<Piece> Bishop::clone() const {
    auto p = std::make_unique<Bishop>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}

// ─── Rook ────────────────────────────────────────────────────────────────────
std::vector<Move> Rook::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;
    addSlidingMoves(board, pos, color_, {{-1,0},{1,0},{0,-1},{0,1}}, moves);
    return moves;
}

std::unique_ptr<Piece> Rook::clone() const {
    auto p = std::make_unique<Rook>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}

// ─── Queen ───────────────────────────────────────────────────────────────────
std::vector<Move> Queen::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;
    addSlidingMoves(board, pos, color_,
        {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}}, moves);
    return moves;
}

std::unique_ptr<Piece> Queen::clone() const {
    auto p = std::make_unique<Queen>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}

// ─── King ────────────────────────────────────────────────────────────────────
std::vector<Move> King::pseudoLegalMoves(const Board& board, Position pos) const {
    std::vector<Move> moves;

    // Normal moves
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr==0 && dc==0) continue;
            Position to{pos.row+dr, pos.col+dc};
            if (!to.isValid()) continue;
            if (board.isEmpty(to) || board.cell(to)->color() != color_)
                moves.push_back({pos, to});
        }

    // Castling
    if (!hasMoved_ && !board.isInCheck(color_)) {
        int ci = (color_ == Color::White) ? 0 : 1;
        int row = pos.row;

        // King-side
        if (board.castlingRights_[ci][1]) {
            bool clear = board.isEmpty({row,5}) && board.isEmpty({row,6});
            bool rookOk = board.cell({row,7}) &&
                          board.cell({row,7})->type() == PieceType::Rook &&
                          !board.cell({row,7})->hasMoved();
            bool safe = !board.isAttackedBy({row,5}, opposite(color_)) &&
                        !board.isAttackedBy({row,6}, opposite(color_));
            if (clear && rookOk && safe)
                moves.push_back({pos, {row,6}, PieceType::Empty, true});
        }
        // Queen-side
        if (board.castlingRights_[ci][0]) {
            bool clear = board.isEmpty({row,1}) && board.isEmpty({row,2}) && board.isEmpty({row,3});
            bool rookOk = board.cell({row,0}) &&
                          board.cell({row,0})->type() == PieceType::Rook &&
                          !board.cell({row,0})->hasMoved();
            bool safe = !board.isAttackedBy({row,3}, opposite(color_)) &&
                        !board.isAttackedBy({row,2}, opposite(color_));
            if (clear && rookOk && safe)
                moves.push_back({pos, {row,2}, PieceType::Empty, true});
        }
    }

    return moves;
}

std::unique_ptr<Piece> King::clone() const {
    auto p = std::make_unique<King>(color_);
    if (hasMoved_) p->setMoved();
    return p;
}