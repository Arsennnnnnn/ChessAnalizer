#ifndef TYPES_H
#define TYPES_H
#include <string>

enum class Color { White, Black, None };
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King, Empty };

struct Position {
    int row, col;
    Position(int r = -1, int c = -1) : row(r), col(c) {}
    bool isValid() const { return row >= 0 && row < 8 && col >= 0 && col < 8; }
    bool operator==(const Position& o) const { return row == o.row && col == o.col; }
    bool operator!=(const Position& o) const { return !(*this == o); }
};

struct Move {
    Position from, to;
    PieceType promotion; // For pawn promotion
    bool isCastling;
    bool isEnPassant;

    Move(Position f = {}, Position t = {}, PieceType promo = PieceType::Empty,
         bool castle = false, bool ep = false)
        : from(f), to(t), promotion(promo), isCastling(castle), isEnPassant(ep) {}

    bool operator==(const Move& o) const {
        return from == o.from && to == o.to && promotion == o.promotion;
    }
};

inline Color opposite(Color c) {
    if (c == Color::White) return Color::Black;
    if (c == Color::Black) return Color::White;
    return Color::None;
}

inline std::string colorToStr(Color c) {
    return c == Color::White ? "White" : (c == Color::Black ? "Black" : "None");
}

inline std::string pieceTypeToChar(PieceType t) {
    switch(t) {
        case PieceType::Pawn:   return "P";
        case PieceType::Knight: return "N";
        case PieceType::Bishop: return "B";
        case PieceType::Rook:   return "R";
        case PieceType::Queen:  return "Q";
        case PieceType::King:   return "K";
        default:                return ".";
    }
}

#endif //TYPES_H