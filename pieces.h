#ifndef PIECES_H
#define PIECES_H
#include "Types.h"
#include <vector>
#include <memory>

class Board;

// ─── Base Piece ────────────────────────────────────────────────────────────────
class Piece {
protected:
    Color color_;
    PieceType type_;
    bool hasMoved_;

public:
    Piece(Color c, PieceType t) : color_(c), type_(t), hasMoved_(false) {}
    virtual ~Piece() = default;

    Color color() const { return color_; }
    PieceType type() const { return type_; }
    bool hasMoved() const { return hasMoved_; }
    void setMoved() { hasMoved_ = true; }

    virtual std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;

    char symbol() const {
        char base;
        switch(type_) {
            case PieceType::Pawn:   base = 'p'; break;
            case PieceType::Knight: base = 'n'; break;
            case PieceType::Bishop: base = 'b'; break;
            case PieceType::Rook:   base = 'r'; break;
            case PieceType::Queen:  base = 'q'; break;
            case PieceType::King:   base = 'k'; break;
            default:                base = '.'; break;
        }
        return (color_ == Color::White) ? (char)toupper(base) : base;
    }
};

// ─── Pawn ──────────────────────────────────────────────────────────────────────
class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c, PieceType::Pawn) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── Knight ────────────────────────────────────────────────────────────────────
class Knight : public Piece {
public:
    Knight(Color c) : Piece(c, PieceType::Knight) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── Bishop ────────────────────────────────────────────────────────────────────
class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c, PieceType::Bishop) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── Rook ──────────────────────────────────────────────────────────────────────
class Rook : public Piece {
public:
    Rook(Color c) : Piece(c, PieceType::Rook) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── Queen ─────────────────────────────────────────────────────────────────────
class Queen : public Piece {
public:
    Queen(Color c) : Piece(c, PieceType::Queen) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── King ──────────────────────────────────────────────────────────────────────
class King : public Piece {
public:
    King(Color c) : Piece(c, PieceType::King) {}
    std::vector<Move> pseudoLegalMoves(const Board& board, Position pos) const override;
    std::unique_ptr<Piece> clone() const override;
};

// ─── Factory ───────────────────────────────────────────────────────────────────
inline std::unique_ptr<Piece> makePiece(PieceType t, Color c) {
    switch(t) {
        case PieceType::Pawn:   return std::make_unique<Pawn>(c);
        case PieceType::Knight: return std::make_unique<Knight>(c);
        case PieceType::Bishop: return std::make_unique<Bishop>(c);
        case PieceType::Rook:   return std::make_unique<Rook>(c);
        case PieceType::Queen:  return std::make_unique<Queen>(c);
        case PieceType::King:   return std::make_unique<King>(c);
        default: return nullptr;
    }
}


#endif //PIECES_H