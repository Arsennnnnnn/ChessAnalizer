#ifndef BOARD_H
#define BOARD_H
#include "Matrix.h"
#include "Pieces.h"
#include <memory>
#include <vector>
#include <string>

// Each cell holds a shared_ptr to a Piece (nullptr = empty)
using Cell = std::shared_ptr<Piece>;

class Board : public Matrix<Cell> {
public:
    Color turn_;                   // whose turn it is
    Position enPassantTarget_;     // en passant target square (-1,-1 if none)
    bool castlingRights_[2][2];    // [color(0=W,1=B)][side(0=Q,1=K)]

    Board();
    Board(const Board& other);
    Board& operator=(const Board& other);

    void setupStandard();
    void clear();

    // Piece access helpers
    Cell& cell(int r, int c) { return at(r, c); }
    const Cell& cell(int r, int c) const { return at(r, c); }
    Cell& cell(Position p) { return at(p.row, p.col); }
    const Cell& cell(Position p) const { return at(p.row, p.col); }

    bool isEmpty(Position p) const { return !cell(p); }
    bool isEmpty(int r, int c) const { return !at(r,c); }

    bool isOccupiedBy(Position p, Color c) const {
        return cell(p) && cell(p)->color() == c;
    }

    // Move execution
    void applyMove(const Move& m);

    // Legal move generation
    std::vector<Move> legalMoves(Color c) const;
    std::vector<Move> pseudoLegalMovesAll(Color c) const;

    // Check/mate/stalemate detection
    bool isInCheck(Color c) const;
    bool isCheckmate(Color c) const;
    bool isStalemate(Color c) const;

    // Position of king
    Position findKing(Color c) const;

    // Is a square attacked by color c?
    bool isAttackedBy(Position p, Color attacker) const;

    // Print board
    void print() const;

    // FEN loading (basic)
    static Board fromFEN(const std::string& fen);
    std::string toFEN() const;

private:
    void deepCopy(const Board& other);
};

#endif //BOARD_H