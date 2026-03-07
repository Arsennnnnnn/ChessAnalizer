#include "Game.h"
#include "Solver.h"
#include <iostream>
#include <algorithm>

Game::Game() : halfmoveClock_(0) {
    board_.setupStandard();
    fenHistory_.push_back(board_.toFEN());
}

Game::Game(const std::string& fen) : halfmoveClock_(0) {
    loadFEN(fen);
}

void Game::reset() {
    board_.setupStandard();
    history_.clear();
    fenHistory_.clear();
    fenHistory_.push_back(board_.toFEN());
    halfmoveClock_ = 0;
}

void Game::loadFEN(const std::string& fen) {
    board_ = Board::fromFEN(fen);
    history_.clear();
    fenHistory_.clear();
    fenHistory_.push_back(board_.toFEN());
    halfmoveClock_ = 0;
}

bool Game::makeMove(const Move& m) {
    auto legal = board_.legalMoves(board_.turn_);
    bool found = std::any_of(legal.begin(), legal.end(), [&](const Move& lm){
        return lm.from == m.from && lm.to == m.to &&
               (m.promotion == PieceType::Empty || lm.promotion == m.promotion);
    });
    if (!found) return false;

    // Fifty move rule counter
    const auto& p = board_.cell(m.from);
    bool isPawnMove = p && p->type() == PieceType::Pawn;
    bool isCapture  = !board_.isEmpty(m.to);
    halfmoveClock_ = (isPawnMove || isCapture) ? 0 : halfmoveClock_ + 1;

    board_.applyMove(m);
    history_.push_back(m);
    fenHistory_.push_back(board_.toFEN());
    return true;
}

bool Game::makeMoveFromString(const std::string& s) {
    if (s.size() < 4) return false;
    int fc = s[0] - 'a', fr = 8 - (s[1] - '0');
    int tc = s[2] - 'a', tr = 8 - (s[3] - '0');
    PieceType promo = PieceType::Empty;
    if (s.size() >= 5) {
        char p = tolower(s[4]);
        if (p=='q') promo=PieceType::Queen;
        else if(p=='r') promo=PieceType::Rook;
        else if(p=='b') promo=PieceType::Bishop;
        else if(p=='n') promo=PieceType::Knight;
    }
    return makeMove(Move{{fr,fc},{tr,tc},promo});
}

bool Game::isFiftyMoveRule() const {
    return halfmoveClock_ >= 100; // 50 full moves = 100 half-moves
}

bool Game::isThreefoldRepetition() const {
    if (fenHistory_.empty()) return false;
    const std::string& cur = fenHistory_.back();
    int count = 0;
    for (const auto& f : fenHistory_)
        if (f == cur) count++;
    return count >= 3;
}

bool Game::isInsufficientMaterial() const {
    // Very basic: K vs K, K+N vs K, K+B vs K
    int total = 0;
    bool hasRookQueenPawn = false;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            const auto& p = board_.at(r,c);
            if (!p) continue;
            total++;
            if (p->type()==PieceType::Rook || p->type()==PieceType::Queen ||
                p->type()==PieceType::Pawn)
                hasRookQueenPawn = true;
        }
    if (hasRookQueenPawn) return false;
    return (total <= 3); // K vs K, K+minor vs K
}

GameResult Game::result() const {
    Color c = board_.turn_;
    if (board_.isCheckmate(c))
        return (c == Color::White) ? GameResult::BlackWin : GameResult::WhiteWin;
    if (board_.isStalemate(c) || isFiftyMoveRule() ||
        isThreefoldRepetition() || isInsufficientMaterial())
        return GameResult::Draw;
    return GameResult::Ongoing;
}

DrawReason Game::drawReason() const {
    Color c = board_.turn_;
    if (board_.isStalemate(c))        return DrawReason::Stalemate;
    if (isFiftyMoveRule())            return DrawReason::FiftyMoveRule;
    if (isThreefoldRepetition())      return DrawReason::ThreefoldRepetition;
    if (isInsufficientMaterial())     return DrawReason::InsufficientMaterial;
    return DrawReason::None;
}

void Game::print() const {
    board_.print();
    std::cout << colorToStr(board_.turn_) << " to move\n";
    auto r = result();
    if (r == GameResult::WhiteWin) std::cout << "Result: White wins!\n";
    else if (r == GameResult::BlackWin) std::cout << "Result: Black wins!\n";
    else if (r == GameResult::Draw) std::cout << "Result: Draw\n";
}

void Game::printHistory() const {
    Board tmp;
    tmp.setupStandard();
    std::cout << "Move history:\n";
    for (size_t i = 0; i < history_.size(); i++) {
        if (i % 2 == 0) std::cout << (i/2+1) << ". ";
        std::cout << Solver::moveToAlgebraic(tmp, history_[i]) << " ";
        tmp.applyMove(history_[i]);
    }
    std::cout << "\n";
}