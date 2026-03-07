#ifndef GAME_H
#define GAME_H
#include "Board.h"
#include <vector>
#include <string>

enum class GameResult { Ongoing, WhiteWin, BlackWin, Draw };
enum class DrawReason { None, Stalemate, FiftyMoveRule, ThreefoldRepetition, InsufficientMaterial };

class Game {
public:
    Game();
    explicit Game(const std::string& fen);

    void reset();
    void loadFEN(const std::string& fen);

    // Returns true if the move was legal and applied
    bool makeMove(const Move& m);
    // Parse and apply a move from simple algebraic (e.g. "e2e4", "e7e8q")
    bool makeMoveFromString(const std::string& s);

    const Board& board() const { return board_; }
    GameResult result() const;
    DrawReason drawReason() const;

    std::vector<Move> legalMoves() const { return board_.legalMoves(board_.turn_); }

    void print() const;
    void printHistory() const;

private:
    Board board_;
    std::vector<Move> history_;
    std::vector<std::string> fenHistory_; // for repetition detection
    int halfmoveClock_; // fifty-move rule

    bool isFiftyMoveRule() const;
    bool isThreefoldRepetition() const;
    bool isInsufficientMaterial() const;
};

#endif // GAME_H