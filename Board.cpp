#include "Board.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Board::Board()
    : Matrix<Cell>(8, 8, nullptr),
      turn_(Color::White),
      enPassantTarget_(-1, -1)
{
    castlingRights_[0][0] = castlingRights_[0][1] = true;
    castlingRights_[1][0] = castlingRights_[1][1] = true;
}

Board::Board(const Board& other)
    : Matrix<Cell>(8, 8, nullptr),
      turn_(other.turn_),
      enPassantTarget_(other.enPassantTarget_)
{
    castlingRights_[0][0] = other.castlingRights_[0][0];
    castlingRights_[0][1] = other.castlingRights_[0][1];
    castlingRights_[1][0] = other.castlingRights_[1][0];
    castlingRights_[1][1] = other.castlingRights_[1][1];
    deepCopy(other);
}

Board& Board::operator=(const Board& other) {
    if (this == &other) return *this;
    Matrix<Cell>::operator=(other);
    turn_ = other.turn_;
    enPassantTarget_ = other.enPassantTarget_;
    castlingRights_[0][0] = other.castlingRights_[0][0];
    castlingRights_[0][1] = other.castlingRights_[0][1];
    castlingRights_[1][0] = other.castlingRights_[1][0];
    castlingRights_[1][1] = other.castlingRights_[1][1];
    deepCopy(other);
    return *this;
}

void Board::deepCopy(const Board& other) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            const auto& p = other.at(r, c);
            at(r, c) = p ? p->clone() : nullptr;
        }
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void Board::clear() {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            at(r, c) = nullptr;
}

void Board::setupStandard() {
    clear();
    // Black pieces on rows 0-1
    auto place = [&](int r, int c, PieceType t, Color col) {
        at(r, c) = makePiece(t, col);
    };
    // Back rank order
    PieceType backRank[] = {
        PieceType::Rook, PieceType::Knight, PieceType::Bishop,
        PieceType::Queen, PieceType::King,
        PieceType::Bishop, PieceType::Knight, PieceType::Rook
    };
    for (int c = 0; c < 8; c++) {
        place(0, c, backRank[c], Color::Black);
        place(1, c, PieceType::Pawn, Color::Black);
        place(6, c, PieceType::Pawn, Color::White);
        place(7, c, backRank[c], Color::White);
    }
    turn_ = Color::White;
    enPassantTarget_ = {-1, -1};
    castlingRights_[0][0] = castlingRights_[0][1] = true;
    castlingRights_[1][0] = castlingRights_[1][1] = true;
}

// ─── Apply Move ───────────────────────────────────────────────────────────────
void Board::applyMove(const Move& m) {
    enPassantTarget_ = {-1, -1};

    auto piece = cell(m.from);
    if (!piece) return;

    int ci = (piece->color() == Color::White) ? 0 : 1;

    // En passant capture
    if (m.isEnPassant) {
        cell(m.to) = cell(m.from);
        cell(m.from) = nullptr;
        int capturedRow = (piece->color() == Color::White) ? m.to.row + 1 : m.to.row - 1;
        at(capturedRow, m.to.col) = nullptr;
    }
    // Castling
    else if (m.isCastling) {
        bool kingSide = m.to.col > m.from.col;
        int rookFromCol = kingSide ? 7 : 0;
        int rookToCol   = kingSide ? 5 : 3;
        int row = m.from.row;
        at(row, m.to.col) = at(row, m.from.col);
        at(row, m.from.col) = nullptr;
        at(row, rookToCol) = at(row, rookFromCol);
        at(row, rookFromCol) = nullptr;
        at(row, rookToCol)->setMoved();
        castlingRights_[ci][0] = castlingRights_[ci][1] = false;
    }
    // Normal / promotion
    else {
        // Set en passant if double pawn push
        if (piece->type() == PieceType::Pawn &&
            std::abs(m.to.row - m.from.row) == 2) {
            int epRow = (m.from.row + m.to.row) / 2;
            enPassantTarget_ = {epRow, m.from.col};
        }

        // Update castling rights if rook moves
        if (piece->type() == PieceType::Rook) {
            if (m.from.col == 0) castlingRights_[ci][0] = false;
            if (m.from.col == 7) castlingRights_[ci][1] = false;
        }
        if (piece->type() == PieceType::King) {
            castlingRights_[ci][0] = castlingRights_[ci][1] = false;
        }

        cell(m.to) = cell(m.from);
        cell(m.from) = nullptr;

        // Promotion
        if (m.promotion != PieceType::Empty) {
            cell(m.to) = makePiece(m.promotion, piece->color());
        }
    }

    if (cell(m.to)) cell(m.to)->setMoved();
    turn_ = opposite(turn_);
}

// ─── Attack detection ────────────────────────────────────────────────────────
bool Board::isAttackedBy(Position p, Color attacker) const {
    // Check attacks directly per piece type to avoid infinite recursion
    // (King.pseudoLegalMoves calls isInCheck which calls isAttackedBy)

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            const auto& piece = at(r, c);
            if (!piece || piece->color() != attacker) continue;

            PieceType t = piece->type();
            Position from{r, c};

            if (t == PieceType::King) {
                // Only normal king moves (no castling — avoids recursion)
                for (int dr = -1; dr <= 1; dr++)
                    for (int dc = -1; dc <= 1; dc++) {
                        if (dr==0 && dc==0) continue;
                        if (from.row+dr == p.row && from.col+dc == p.col)
                            return true;
                    }
            } else {
                auto moves = piece->pseudoLegalMoves(*this, from);
                for (const auto& mv : moves) {
                    if (mv.to == p) return true;
                }
            }
        }
    }
    return false;
}

Position Board::findKing(Color c) const {
    for (int r = 0; r < 8; r++)
        for (int col = 0; col < 8; col++)
            if (at(r, col) && at(r, col)->type() == PieceType::King &&
                at(r, col)->color() == c)
                return {r, col};
    return {-1, -1};
}

bool Board::isInCheck(Color c) const {
    Position kp = findKing(c);
    if (!kp.isValid()) return false;
    return isAttackedBy(kp, opposite(c));
}

// ─── Legal move generation ───────────────────────────────────────────────────
std::vector<Move> Board::pseudoLegalMovesAll(Color c) const {
    std::vector<Move> result;
    for (int r = 0; r < 8; r++)
        for (int col = 0; col < 8; col++) {
            const auto& p = at(r, col);
            if (!p || p->color() != c) continue;
            auto moves = p->pseudoLegalMoves(*this, {r, col});
            result.insert(result.end(), moves.begin(), moves.end());
        }
    return result;
}

std::vector<Move> Board::legalMoves(Color c) const {
    auto pseudo = pseudoLegalMovesAll(c);
    std::vector<Move> legal;
    for (const auto& mv : pseudo) {
        Board copy(*this);
        copy.applyMove(mv);
        // After the move it's opponent's turn, but we check if c's king is in check
        if (!copy.isInCheck(c)) {
            legal.push_back(mv);
        }
    }
    return legal;
}

bool Board::isCheckmate(Color c) const {
    return isInCheck(c) && legalMoves(c).empty();
}

bool Board::isStalemate(Color c) const {
    return !isInCheck(c) && legalMoves(c).empty();
}

// ─── Print ────────────────────────────────────────────────────────────────────
void Board::print() const {
    std::cout << "\n  a b c d e f g h\n";
    for (int r = 0; r < 8; r++) {
        std::cout << (8 - r) << " ";
        for (int c = 0; c < 8; c++) {
            const auto& p = at(r, c);
            std::cout << (p ? p->symbol() : '.') << ' ';
        }
        std::cout << (8 - r) << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

// ─── FEN ─────────────────────────────────────────────────────────────────────
Board Board::fromFEN(const std::string& fen) {
    Board b;
    b.clear();

    std::istringstream ss(fen);
    std::string piecePart, turnPart, castlePart, epPart;
    ss >> piecePart >> turnPart >> castlePart >> epPart;

    int row = 0, col = 0;
    for (char ch : piecePart) {
        if (ch == '/') { row++; col = 0; continue; }
        if (isdigit(ch)) { col += (ch - '0'); continue; }
        Color color = isupper(ch) ? Color::White : Color::Black;
        char lower = tolower(ch);
        PieceType t = PieceType::Empty;
        if (lower=='p') t=PieceType::Pawn;
        else if (lower=='n') t=PieceType::Knight;
        else if (lower=='b') t=PieceType::Bishop;
        else if (lower=='r') t=PieceType::Rook;
        else if (lower=='q') t=PieceType::Queen;
        else if (lower=='k') t=PieceType::King;
        if (t != PieceType::Empty) b.at(row, col) = makePiece(t, color);
        col++;
    }

    b.turn_ = (turnPart == "w") ? Color::White : Color::Black;

    b.castlingRights_[0][0]=b.castlingRights_[0][1]=false;
    b.castlingRights_[1][0]=b.castlingRights_[1][1]=false;
    for (char ch : castlePart) {
        if (ch=='K') b.castlingRights_[0][1]=true;
        if (ch=='Q') b.castlingRights_[0][0]=true;
        if (ch=='k') b.castlingRights_[1][1]=true;
        if (ch=='q') b.castlingRights_[1][0]=true;
    }

    if (epPart != "-") {
        int epc = epPart[0] - 'a';
        int epr = 8 - (epPart[1] - '0');
        b.enPassantTarget_ = {epr, epc};
    }

    return b;
}

std::string Board::toFEN() const {
    std::string fen;
    for (int r = 0; r < 8; r++) {
        int empty = 0;
        for (int c = 0; c < 8; c++) {
            const auto& p = at(r, c);
            if (!p) { empty++; }
            else {
                if (empty) { fen += ('0' + empty); empty = 0; }
                fen += p->symbol();
            }
        }
        if (empty) fen += ('0' + empty);
        if (r < 7) fen += '/';
    }
    fen += ' ';
    fen += (turn_ == Color::White) ? 'w' : 'b';
    fen += ' ';
    std::string castle;
    if (castlingRights_[0][1]) castle += 'K';
    if (castlingRights_[0][0]) castle += 'Q';
    if (castlingRights_[1][1]) castle += 'k';
    if (castlingRights_[1][0]) castle += 'q';
    fen += castle.empty() ? "-" : castle;
    fen += ' ';
    if (enPassantTarget_.isValid())
        fen += (char)('a' + enPassantTarget_.col), fen += (char)('0' + (8 - enPassantTarget_.row));
    else
        fen += '-';
    return fen;
}