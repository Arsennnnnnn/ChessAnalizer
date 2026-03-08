#include "Board.h"
#include "Solver.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <cmath>

static bool parsePiece(const std::string& token, Color& color, PieceType& type, Position& pos) {
    if (token.size() < 3) return false;
    char pc = token[0];
    color = isupper(pc) ? Color::White : Color::Black;
    char lower = tolower(pc);
    if      (lower == 'k') type = PieceType::King;
    else if (lower == 'q') type = PieceType::Queen;
    else if (lower == 'r') type = PieceType::Rook;
    else if (lower == 'b') type = PieceType::Bishop;
    else if (lower == 'n') type = PieceType::Knight;
    else if (lower == 'p') type = PieceType::Pawn;
    else return false;
    char file = token[1];
    char rank = token[2];
    if (file < 'a' || file > 'h') return false;
    if (rank < '1' || rank > '8') return false;
    pos.col = file - 'a';
    pos.row = 8 - (rank - '0');
    return true;
}


static void printHelp() {
    std::cout << "\n"
"|==========================================================|\n"
"|            Chess Mate Solver - Input Guide               |\n"
"|==========================================================|\n"
"|  Enter pieces separated by spaces.                       |\n"
"|  Optionally add 'w' or 'b' for whose turn it is.         |\n"
"|                                                          |\n"
"|  Piece format:  <Piece><file><rank>                      |\n"
"|    Uppercase = White piece,  lowercase = Black piece     |\n"
"|                                                          |\n"
"|  Pieces:  K/k=King   Q/q=Queen  R/r=Rook                 |\n"
"|           B/b=Bishop N/n=Knight P/p=Pawn                 |\n"
"|                                                          |\n"
"|  Examples:                                               |\n"
"|    Ke1 Qd1 ke8 rb8          (White to move, default)     |\n"
"|    Ke1 Qd1 ke8 rb8 b        (Black to move)              |\n"
"|    Ka6 Rb6 ka8              (already mated - detected)   |\n"
"|==========================================================|\n\n";
}

static void solvePosition(const Board& board) {
    Solver solver;
    board.print();

    for (Color side : {Color::White, Color::Black}) {
        if (board.isCheckmate(side)) {
            std::cout << ">>> This position is already checkmate! "
                      << colorToStr(opposite(side)) << " wins.\n\n";
            return;
        }
        if (board.isStalemate(side)) {
            std::cout << ">>> This position is already stalemate — it is a draw.\n\n";
            return;
        }
    }

    std::cout << colorToStr(board.turn_) << " to move\n\n";
    std::cout << "Searching for forced mate (up to 3 moves)...\n\n";

    bool foundAny = false;
    for (int depth = 1; depth <= 3; depth++) {
        auto t0    = std::chrono::steady_clock::now();
        auto lines = solver.findMateInN(board, 2*depth - 1);
        double ms  = std::chrono::duration<double, std::milli>(
                         std::chrono::steady_clock::now() - t0).count();

        if (!lines.empty()) {
            std::cout << ">>> Mate in " << depth << "!  ("
                      << lines.size() << " solution(s), " << ms << " ms)\n\n";
            std::cout << "  Best line:  ";
            Solver::printSolveLine(board, lines[0], board.turn_);
            if (lines.size() > 1) {
                std::cout << "  Other mating first move(s): ";
                for (size_t i = 1; i < lines.size(); i++) {
                    std::cout << Solver::moveToAlgebraic(board, lines[i].moves[0]);
                    if (i + 1 < lines.size()) std::cout << ",  ";
                }
                std::cout << "\n";
            }
            foundAny = true;
            break;
        } else {
            std::cout << "  No mate in " << depth << "  (" << ms << " ms)\n";
        }
    }

    if (!foundAny)
        std::cout << "\nNo forced mate within 3 moves from this position.\n";
    std::cout << "\n";
}

int main() {
    printHelp();
    std::string line;
    while (true) {
        std::cout << "Position> ";
        if (!std::getline(std::cin, line)) break;
        while (!line.empty() && isspace((unsigned char)line.front())) line.erase(line.begin());
        while (!line.empty() && isspace((unsigned char)line.back()))  line.pop_back();
        if (line.empty()) continue;

        if (line == "quit" || line == "q" || line == "exit") { std::cout << "Goodbye.\n"; break; }
        if (line == "help") { printHelp(); continue; }

        // FEN mode
        if (line.size() > 4 && line.substr(0, 4) == "fen ") {
            Board b = Board::fromFEN(line.substr(4));
            solvePosition(b);
            continue;
        }

        // Manual piece-placement mode
        Board board;
        board.clear();
        board.castlingRights_[0][0] = board.castlingRights_[0][1] = false;
        board.castlingRights_[1][0] = board.castlingRights_[1][1] = false;
        board.turn_ = Color::White; // default

        std::istringstream ss(line);
        std::string token;
        bool parseOk = true;

        while (ss >> token) {
            if (token == "w" || token == "W") { board.turn_ = Color::White; continue; }
            if (token == "b" || token == "B") { board.turn_ = Color::Black; continue; }

            Color c; PieceType t; Position pos;
            if (!parsePiece(token, c, t, pos)) {
                std::cout << "  Unknown token: '" << token << "'  (type 'help' for format)\n\n";
                parseOk = false; break;
            }

            // Two pieces on the same square
            if (!board.isEmpty(pos)) {
                std::cout << "  Error: square " << (char)('a' + pos.col) << (8 - pos.row)
                          << " is already occupied — two pieces cannot share a square.\n\n";
                parseOk = false; break;
            }

            // Pawn on 1st or 8th rank
            if (t == PieceType::Pawn && (pos.row == 0 || pos.row == 7)) {
                std::cout << "  Error: '" << token << "' — pawns cannot stand on the 1st or 8th rank.\n\n";
                parseOk = false; break;
            }

            board.cell(pos) = makePiece(t, c);
        }
        if (!parseOk) continue;

        // Both kings must be present
        if (!board.findKing(Color::White).isValid() || !board.findKing(Color::Black).isValid()) {
            std::cout << "  Error: both kings must be on the board (e.g. Ke1 ke8).\n\n";
            continue;
        }

        // Only one king per side
        int wKings = 0, bKings = 0;
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++) {
                const auto& p = board.at(r, c);
                if (p && p->type() == PieceType::King) {
                    if (p->color() == Color::White) wKings++;
                    else bKings++;
                }
            }
        if (wKings > 1 || bKings > 1) {
            std::cout << "  Error: each side can only have one king.\n\n";
            continue;
        }

        // Kings cannot be adjacent
        {
            Position wk = board.findKing(Color::White);
            Position bk = board.findKing(Color::Black);
            if (std::abs(wk.row - bk.row) <= 1 && std::abs(wk.col - bk.col) <= 1) {
                std::cout << "  Error: the two kings cannot be adjacent — illegal position.\n\n";
                continue;
            }
        }

        // The side that just moved cannot be in check
        {
            Color justMoved = opposite(board.turn_);
            if (board.isInCheck(justMoved) && !board.isCheckmate(justMoved)) {
                std::cout << "  Error: " << colorToStr(justMoved)
                          << " (the side that just moved) cannot be left in check — illegal position.\n\n";
                continue;
            }
        }

        solvePosition(board);
    }
    return 0;
}