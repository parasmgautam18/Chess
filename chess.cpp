#include <bits/stdc++.h>
#include "raylib.h"
#include <cstdint>

using namespace std;

#define Bitboard uint64_t
const int BOARD_SIZE = 8;
const int TILE_SIZE = 100; // 800x800 window
const int INF = 1000000;
const int MATE_SCORE = 100000;
const int SEARCH_DEPTH = 5;

struct ChessGame {
    Bitboard whitePawns   = 0x00FF000000000000ULL;
    Bitboard whiteRooks   = 0x8100000000000000ULL;
    Bitboard whiteKnights = 0x4200000000000000ULL;
    Bitboard whiteBishops = 0x2400000000000000ULL;
    Bitboard whiteQueens  = 0x0800000000000000ULL;
    Bitboard whiteKing    = 0x1000000000000000ULL;

    Bitboard blackPawns   = 0x000000000000FF00ULL;
    Bitboard blackRooks   = 0x0000000000000081ULL;
    Bitboard blackKnights = 0x0000000000000042ULL;
    Bitboard blackBishops = 0x0000000000000024ULL;
    Bitboard blackQueens  = 0x0000000000000008ULL;
    Bitboard blackKing    = 0x0000000000000010ULL;

    bool whiteTurn = true;
    int enPassantSquare = -1;

    // Castling Rights
    bool whiteCastleKingSide  = true;
    bool whiteCastleQueenSide = true;
    bool blackCastleKingSide  = true;
    bool blackCastleQueenSide = true;

    Bitboard whitePieces() const {
        return whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueens | whiteKing;
    }
    Bitboard blackPieces() const {
        return blackPawns | blackRooks | blackKnights | blackBishops | blackQueens | blackKing;
    }
    Bitboard allPieces() const {
        return whitePieces() | blackPieces();
    }
};

// --- Bit Manipulation Helpers ---
inline bool getBit(Bitboard bb, int sq) {
    return (bb & (1ULL << sq)) != 0;
}

inline void setBit(Bitboard &bb, int sq) {
    bb |= (1ULL << sq);
}

inline void clearBit(Bitboard &bb, int sq) {
    bb &= ~(1ULL << sq);
}

int getPieceAtSquare(const ChessGame &game, int sq) {
    if (getBit(game.whitePawns, sq))   return  1;
    if (getBit(game.whiteKnights, sq)) return  2;
    if (getBit(game.whiteBishops, sq)) return  3;
    if (getBit(game.whiteRooks, sq))   return  4;
    if (getBit(game.whiteQueens, sq))  return  5;
    if (getBit(game.whiteKing, sq))    return  6;

    if (getBit(game.blackPawns, sq))   return -1;
    if (getBit(game.blackKnights, sq)) return -2;
    if (getBit(game.blackBishops, sq)) return -3;
    if (getBit(game.blackRooks, sq))   return -4;
    if (getBit(game.blackQueens, sq))  return -5;
    if (getBit(game.blackKing, sq))    return -6;
    return 0;
}

void removePieceAtSquare(ChessGame &game, int sq) {
    Bitboard mask = ~(1ULL << sq);
    game.whitePawns   &= mask;
    game.whiteKnights &= mask;
    game.whiteBishops &= mask;
    game.whiteRooks   &= mask;
    game.whiteQueens  &= mask;
    game.whiteKing    &= mask;

    game.blackPawns   &= mask;
    game.blackKnights &= mask;
    game.blackBishops &= mask;
    game.blackRooks   &= mask;
    game.blackQueens  &= mask;
    game.blackKing    &= mask;
}

void setPieceAtSquare(ChessGame &game, int sq, int piece) {
    removePieceAtSquare(game, sq);
    switch (piece) {
        case  1: setBit(game.whitePawns, sq);   break;
        case  2: setBit(game.whiteKnights, sq); break;
        case  3: setBit(game.whiteBishops, sq); break;
        case  4: setBit(game.whiteRooks, sq);   break;
        case  5: setBit(game.whiteQueens, sq);  break;
        case  6: setBit(game.whiteKing, sq);    break;

        case -1: setBit(game.blackPawns, sq);   break;
        case -2: setBit(game.blackKnights, sq); break;
        case -3: setBit(game.blackBishops, sq); break;
        case -4: setBit(game.blackRooks, sq);   break;
        case -5: setBit(game.blackQueens, sq);  break;
        case -6: setBit(game.blackKing, sq);    break;
    }
}

void movePiece(ChessGame &game, int fromSq, int toSq) {
    if (fromSq == toSq) return;

    int movingPiece = getPieceAtSquare(game, fromSq);
    if (movingPiece == 0) return;

    bool isWhite = movingPiece > 0;

    // 1. Handle Castling
    if (movingPiece == 6) {
        if (fromSq == 60 && toSq == 62) {
            removePieceAtSquare(game, 63);
            setPieceAtSquare(game, 61, 4);
        } 
        else if (fromSq == 60 && toSq == 58) {
            removePieceAtSquare(game, 56);
            setPieceAtSquare(game, 59, 4);
        }
    } 
    else if (movingPiece == -6) {
        if (fromSq == 4 && toSq == 6) {
            removePieceAtSquare(game, 7);
            setPieceAtSquare(game, 5, -4);
        } 
        else if (fromSq == 4 && toSq == 2) {
            removePieceAtSquare(game, 0);
            setPieceAtSquare(game, 3, -4);
        }
    }

    // En Passant capture removal
    if (abs(movingPiece) == 1 && toSq == game.enPassantSquare) {
        int captureSq = isWhite ? (toSq + 8) : (toSq - 8);
        removePieceAtSquare(game, captureSq);
    }

    // 2. Move the piece itself
    removePieceAtSquare(game, fromSq);
    setPieceAtSquare(game, toSq, movingPiece);

    // 3. Update Castling Rights
    if (fromSq == 60) { game.whiteCastleKingSide = false; game.whiteCastleQueenSide = false; }
    if (fromSq == 4)  { game.blackCastleKingSide = false; game.blackCastleQueenSide = false; }

    if (fromSq == 63 || toSq == 63) game.whiteCastleKingSide  = false;
    if (fromSq == 56 || toSq == 56) game.whiteCastleQueenSide = false;
    if (fromSq == 7  || toSq == 7)  game.blackCastleKingSide  = false;
    if (fromSq == 0  || toSq == 0)  game.blackCastleQueenSide = false;

    // 4. Update En Passant square
    if (abs(movingPiece) == 1 && abs(fromSq - toSq) == 16) {
        game.enPassantSquare = (fromSq + toSq) / 2;
    } 
    else {
        game.enPassantSquare = -1;
    }
}

// --- Move Generation ---
Bitboard getPseudoLegalMoves(const ChessGame &game, int sq) {
    Bitboard moves = 0ULL;
    int piece = getPieceAtSquare(game, sq);
    if (piece == 0) return 0ULL;

    bool isWhite = (piece > 0);
    int type = abs(piece);
    int r = sq / 8;
    int c = sq % 8;

    Bitboard friendly = isWhite ? game.whitePieces() : game.blackPieces();
    Bitboard enemy    = isWhite ? game.blackPieces() : game.whitePieces();
    Bitboard occupied = friendly | enemy;

    if (type == 1) {
        int dir = isWhite ? -1 : 1;
        int startRow = isWhite ? 6 : 1;
        int nextR = r + dir;
        if (nextR >= 0 && nextR < 8) {
            int oneStepSq = nextR * 8 + c;
            if (!getBit(occupied, oneStepSq)) {
                setBit(moves, oneStepSq);
                int twoStepR = r + 2 * dir;
                int twoStepSq = twoStepR * 8 + c;
                if (r == startRow && !getBit(occupied, twoStepSq)) {
                    setBit(moves, twoStepSq);
                }
            }
            for (int dc : {-1, 1}) {
                int capC = c + dc;
                if (capC >= 0 && capC < 8) {
                    int capSq = nextR * 8 + capC;
                    if (getBit(enemy, capSq) || capSq == game.enPassantSquare) {
                        setBit(moves, capSq);
                    }
                }
            }
        }
    } 
    else if (type == 2) {
        const int dr[8] = {-2, -2, -1, -1,  1,  1,  2,  2};
        const int dc[8] = {-1,  1, -2,  2, -2,  2, -1,  1};
        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                int targetSq = nr * 8 + nc;
                if (!getBit(friendly, targetSq)) setBit(moves, targetSq);
            }
        }
    } 
    else if (type == 6) {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int nr = r + dr, nc = c + dc;
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    int targetSq = nr * 8 + nc;
                    if (!getBit(friendly, targetSq)) setBit(moves, targetSq);
                }
            }
        }
    } 
    else {
        const int dr[8] = {-1,  1,  0,  0, -1, -1,  1,  1};
        const int dc[8] = { 0,  0, -1,  1, -1,  1, -1,  1};
        int startDir = (type == 3) ? 4 : 0;
        int endDir   = (type == 4) ? 4 : 8;

        for (int d = startDir; d < endDir; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                int targetSq = nr * 8 + nc;
                if (getBit(friendly, targetSq)) break;
                setBit(moves, targetSq);
                if (getBit(enemy, targetSq)) break;
                nr += dr[d];
                nc += dc[d];
            }
        }
    }

    return moves;
}

int getKingSquare(const ChessGame &game, bool whiteKing) {
    Bitboard kingBB = whiteKing ? game.whiteKing : game.blackKing;
    if (kingBB == 0ULL) return -1;
    return __builtin_ctzll(kingBB);
}

// Fast direct attack detector for the AI search tree
bool isSquareAttacked(const ChessGame &game, int sq, bool byWhite) {
    int r = sq / 8;
    int c = sq % 8;

    // 1. Attacked by Pawns
    int pawnRow = byWhite ? (r + 1) : (r - 1);
    Bitboard enemyPawns = byWhite ? game.whitePawns : game.blackPawns;
    if (pawnRow >= 0 && pawnRow < 8) {
        if (c > 0 && getBit(enemyPawns, pawnRow * 8 + (c - 1))) return true;
        if (c < 7 && getBit(enemyPawns, pawnRow * 8 + (c + 1))) return true;
    }

    // 2. Attacked by Knights
    Bitboard enemyKnights = byWhite ? game.whiteKnights : game.blackKnights;
    const int kdr[8] = {-2, -2, -1, -1,  1,  1,  2,  2};
    const int kdc[8] = {-1,  1, -2,  2, -2,  2, -1,  1};
    for (int i = 0; i < 8; i++) {
        int nr = r + kdr[i], nc = c + kdc[i];
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (getBit(enemyKnights, nr * 8 + nc)) return true;
        }
    }

    // 3. Attacked by King
    Bitboard enemyKing = byWhite ? game.whiteKing : game.blackKing;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                if (getBit(enemyKing, nr * 8 + nc)) return true;
            }
        }
    }

    // 4. Attacked by Rooks / Queens (Orthogonal) & Bishops / Queens (Diagonal)
    Bitboard occupied     = game.allPieces();
    Bitboard orthSliders  = byWhite ? (game.whiteRooks | game.whiteQueens) : (game.blackRooks | game.blackQueens);
    Bitboard diagSliders  = byWhite ? (game.whiteBishops | game.whiteQueens) : (game.blackBishops | game.blackQueens);

    const int sdr[8] = {-1,  1,  0,  0, -1, -1,  1,  1};
    const int sdc[8] = { 0,  0, -1,  1, -1,  1, -1,  1};

    for (int d = 0; d < 8; d++) {
        int nr = r + sdr[d], nc = c + sdc[d];
        Bitboard targetSliders = (d < 4) ? orthSliders : diagSliders;
        while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            int tSq = nr * 8 + nc;
            if (getBit(occupied, tSq)) {
                if (getBit(targetSliders, tSq)) return true;
                break;
            }
            nr += sdr[d];
            nc += sdc[d];
        }
    }

    return false;
}

bool isInCheck(const ChessGame &game, bool whiteKing) {
    int kingSq = getKingSquare(game, whiteKing);
    if (kingSq == -1) return true;
    return isSquareAttacked(game, kingSq, !whiteKing);
}

Bitboard getLegalMoves(const ChessGame &game, int sq) {
    Bitboard pseudoMoves = getPseudoLegalMoves(game, sq);
    Bitboard legalMoves = 0ULL;

    int piece = getPieceAtSquare(game, sq);
    if (piece == 0) return 0ULL;
    bool isWhite = (piece > 0);

    Bitboard tempMask = pseudoMoves;
    while (tempMask) {
        int targetSq = __builtin_ctzll(tempMask);
        tempMask &= tempMask - 1;

        ChessGame tempGame = game;
        movePiece(tempGame, sq, targetSq);

        if (!isInCheck(tempGame, isWhite)) {
            setBit(legalMoves, targetSq);
        }
    }

    // --- CASTLING GENERATION ---
    Bitboard occupied = game.allPieces();

    if (piece == 6 && sq == 60 && !isInCheck(game, true)) {
        if (game.whiteCastleKingSide && getBit(game.whiteRooks, 63) &&
            !getBit(occupied, 61) && !getBit(occupied, 62)) {
            if (!isSquareAttacked(game, 61, false) && !isSquareAttacked(game, 62, false)) {
                setBit(legalMoves, 62);
            }
        }
        if (game.whiteCastleQueenSide && getBit(game.whiteRooks, 56) &&
            !getBit(occupied, 57) && !getBit(occupied, 58) && !getBit(occupied, 59)) {
            if (!isSquareAttacked(game, 59, false) && !isSquareAttacked(game, 58, false)) {
                setBit(legalMoves, 58);
            }
        }
    }

    if (piece == -6 && sq == 4 && !isInCheck(game, false)) {
        if (game.blackCastleKingSide && getBit(game.blackRooks, 7) &&
            !getBit(occupied, 5) && !getBit(occupied, 6)) {
            if (!isSquareAttacked(game, 5, true) && !isSquareAttacked(game, 6, true)) {
                setBit(legalMoves, 6);
            }
        }
        if (game.blackCastleQueenSide && getBit(game.blackRooks, 0) &&
            !getBit(occupied, 1) && !getBit(occupied, 2) && !getBit(occupied, 3)) {
            if (!isSquareAttacked(game, 3, true) && !isSquareAttacked(game, 2, true)) {
                setBit(legalMoves, 2);
            }
        }
    }

    return legalMoves;
}

bool hasAnyLegalMoves(const ChessGame &game, bool isWhite) {
    Bitboard pieces = isWhite ? game.whitePieces() : game.blackPieces();
    while (pieces) {
        int sq = __builtin_ctzll(pieces);
        pieces &= pieces - 1;
        if (getLegalMoves(game, sq) != 0ULL) return true;
    }
    return false;
}

//                          CHESS ENGINE (AI BOT)

struct EngineMove {
    int fromSq = -1;
    int toSq = -1;
    int promoPiece = 0;
    int score = 0;  
};

// Piece-Square Tables (from White's perspective, a8 = index 0, h1 = index 63)
const int pawnPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

const int knightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int rookPST[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

const int queenPST[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int kingPST[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int pieceValues[7] = { 0, 100, 320, 330, 500, 900, 20000 };

// Static Evaluation from current side-to-move's perspective (for Negamax)
int evaluateBoard(const ChessGame &game) {
    int score = 0;

    auto evalBitboard = [&](Bitboard bb, int pieceType, const int pst[64], bool isWhite) {
        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;
            int tableSq = isWhite ? sq : (sq ^ 56);
            int val = pieceValues[pieceType] + pst[tableSq];
            score += isWhite ? val : -val;
        }
    };

    evalBitboard(game.whitePawns,   1, pawnPST,   true);
    evalBitboard(game.whiteKnights, 2, knightPST, true);
    evalBitboard(game.whiteBishops, 3, bishopPST, true);
    evalBitboard(game.whiteRooks,   4, rookPST,   true);
    evalBitboard(game.whiteQueens,  5, queenPST,  true);
    evalBitboard(game.whiteKing,    6, kingPST,   true);

    evalBitboard(game.blackPawns,   1, pawnPST,   false);
    evalBitboard(game.blackKnights, 2, knightPST, false);
    evalBitboard(game.blackBishops, 3, bishopPST, false);
    evalBitboard(game.blackRooks,   4, rookPST,   false);
    evalBitboard(game.blackQueens,  5, queenPST,  false);
    evalBitboard(game.blackKing,    6, kingPST,   false);

    // Bishop pair bonus
    if (__builtin_popcountll(game.whiteBishops) >= 2) score += 30;
    if (__builtin_popcountll(game.blackBishops) >= 2) score -= 30;

    return game.whiteTurn ? score : -score;
}

// Applies an EngineMove to a board state and flips the turn
void applyEngineMove(ChessGame &game, const EngineMove &m) {
    movePiece(game, m.fromSq, m.toSq);
    if (m.promoPiece != 0) {
        setPieceAtSquare(game, m.toSq, m.promoPiece);
    }
    game.whiteTurn = !game.whiteTurn;
}

// Bulk Legal Move Generator with MVV-LVA Move Ordering
vector<EngineMove> generateAllMoves(const ChessGame &game, bool capturesOnly = false) {
    vector<EngineMove> moveList;
    moveList.reserve(48);

    bool isWhite = game.whiteTurn;
    Bitboard friendlyPieces = isWhite ? game.whitePieces() : game.blackPieces();
    Bitboard enemyPieces    = isWhite ? game.blackPieces() : game.whitePieces();

    while (friendlyPieces) {
        int fromSq = __builtin_ctzll(friendlyPieces);
        friendlyPieces &= friendlyPieces - 1;

        int attacker = abs(getPieceAtSquare(game, fromSq));
        Bitboard legalTargets = getLegalMoves(game, fromSq);

        while (legalTargets) {
            int toSq = __builtin_ctzll(legalTargets);
            legalTargets &= legalTargets - 1;

            bool isCapture = getBit(enemyPieces, toSq) || (attacker == 1 && toSq == game.enPassantSquare);
            bool isPromotion = (attacker == 1) && (toSq < 8 || toSq >= 56);

            if (capturesOnly && !isCapture && !isPromotion) continue;

            int victim = abs(getPieceAtSquare(game, toSq));
            if (attacker == 1 && toSq == game.enPassantSquare) victim = 1;

            int baseScore = 0;
            if(isCapture){
                baseScore = 10000 + (pieceValues[victim] * 10) - pieceValues[attacker];
            }

            if(isPromotion){
                for (int promo : { 5, 4, 3, 2 }) {
                    EngineMove m;
                    m.fromSq = fromSq;
                    m.toSq = toSq;
                    m.promoPiece = isWhite ? promo : -promo;
                    m.score = baseScore + pieceValues[promo] + 5000;
                    moveList.push_back(m);
                }
            } 
            else {
                EngineMove m;
                m.fromSq = fromSq;
                m.toSq = toSq;
                m.promoPiece = 0;
                m.score = baseScore;
                moveList.push_back(m);
            }
        }
    }

    // Sort highest-scoring moves first for maximum Alpha-Beta pruning
    sort(moveList.begin(), moveList.end(), [](const EngineMove &a, const EngineMove &b) {
        return a.score > b.score;
    });

    return moveList;
}

// Quiescence Search: searches captures past depth 0 to prevent tactical blunders
int quiescenceSearch(const ChessGame &game, int alpha, int beta) {
    int standPat = evaluateBoard(game);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    vector<EngineMove> captures = generateAllMoves(game, true);
    for (const EngineMove &m : captures) {
        ChessGame nextGame = game;
        applyEngineMove(nextGame, m);

        int score = -quiescenceSearch(nextGame, -beta, -alpha);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// Negamax Search with Alpha-Beta Pruning
int negamax(const ChessGame &game, int depth, int alpha, int beta, int ply) {
    if (depth == 0) {
        return quiescenceSearch(game, alpha, beta);
    }

    vector<EngineMove> moves = generateAllMoves(game, false);

    if (moves.empty()) {
        if (isInCheck(game, game.whiteTurn)) {
            return -MATE_SCORE + ply; 
        }
        return 0; 
    }

    int maxScore = -INF;
    for (const EngineMove &m : moves) {
        ChessGame nextGame = game;
        applyEngineMove(nextGame, m);

        int score = -negamax(nextGame, depth - 1, -beta, -alpha, ply + 1);
        if (score > maxScore) maxScore = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }

    return maxScore;
}

// Root Search: picks the best move for the AI
EngineMove findBestMove(const ChessGame &game, int depth) {
    vector<EngineMove> moves = generateAllMoves(game, false);
    EngineMove bestMove;
    if (moves.empty()) return bestMove;

    bestMove = moves[0];
    int bestScore = -INF;
    int alpha = -INF;
    int beta = INF;

    for (const EngineMove &m : moves) {
        ChessGame nextGame = game;
        applyEngineMove(nextGame, m);

        int score = -negamax(nextGame, depth - 1, -beta, -alpha, 1);
        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return bestMove;
}

// --- GAME STATES ---
enum GameState { MENU, PLAYING };

int main() {
    InitWindow(BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE, "Bitboard Chess C++ vs AI Engine");
    SetTargetFPS(60);

    map<int, Texture2D> pieceTextures = {
        { 1, LoadTexture("assets/wP.png")},
        { 2, LoadTexture("assets/wN.png")},
        { 3, LoadTexture("assets/wB.png")},
        { 4, LoadTexture("assets/wR.png")},
        { 5, LoadTexture("assets/wQ.png")},
        { 6, LoadTexture("assets/wK.png")},

        {-1, LoadTexture("assets/bP.png")},
        {-2, LoadTexture("assets/bN.png")},
        {-3, LoadTexture("assets/bB.png")},
        {-4, LoadTexture("assets/bR.png")},
        {-5, LoadTexture("assets/bQ.png")},
        {-6, LoadTexture("assets/bK.png")}
    };

    for (auto &pair : pieceTextures) {
        SetTextureFilter(pair.second, TEXTURE_FILTER_BILINEAR);
    }

    GameState currentState = MENU;
    bool playAsWhite = true;

    ChessGame game;
    int selectedSquare = -1;
    int promotionSquare = -1;
    int lastMoveFrom = -1;
    int lastMoveTo = -1;
    Bitboard currentLegalMoves = 0ULL;

    bool gameOver = false;
    double botTurnStartTime = 0.0;
    const double BOT_DELAY = 2.0;
    string winMessage = "";

    const int promoTypes[4] = { 5, 4, 3, 2 };
    const int menuStartX = 2 * TILE_SIZE;
    const int menuStartY = 4 * TILE_SIZE - 50;

    Color lightSquare    = { 238, 238, 210, 255 };
    Color darkSquare     = { 118, 150,  86, 255 };
    Color highlightColor = { 246, 246, 105, 255 };
    Color lastMoveColor  = { 186, 202,  68, 255 };
    Color checkColor     = { 235,  64,  52, 255 };
    Color moveIndicator  = {   0,   0,   0,  60 };

    auto checkGameEnd = [&]() {
        if (!hasAnyLegalMoves(game, game.whiteTurn)) {
            gameOver = true;
            if (isInCheck(game, game.whiteTurn)) {
                winMessage = game.whiteTurn ? "Black Won by Checkmate!" : "White Won by Checkmate!";
            } 
            else {
                winMessage = "Draw by Stalemate!";
            }
        }
    };

    while (!WindowShouldClose()) {
        // --- 0. COLOR SELECTION MENU ---
        if (currentState == MENU) {
            Vector2 mousePoint = { (float)GetMouseX(), (float)GetMouseY() };
            Rectangle btnWhite = { 180, 320, 180, 200 };
            Rectangle btnBlack = { 440, 320, 180, 200 };

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePoint, btnWhite)) {
                    playAsWhite = true;
                    game = ChessGame();
                    selectedSquare = promotionSquare = lastMoveFrom = lastMoveTo = -1;
                    gameOver = false;
                    currentState = PLAYING;
                } 
                else if (CheckCollisionPointRec(mousePoint, btnBlack)) {
                    playAsWhite = false;
                    game = ChessGame();
                    selectedSquare = promotionSquare = lastMoveFrom = lastMoveTo = -1;
                    gameOver = false;
                    botTurnStartTime = GetTime(); // Start 2-second timer for White AI's first move
                    currentState = PLAYING;
                }
            }

            BeginDrawing();
            ClearBackground({ 35, 35, 35, 255 });

            const char* title = "CHOOSE YOUR SIDE VS AI";
            int titleWidth = MeasureText(title, 40);
            DrawText(title, (BOARD_SIZE * TILE_SIZE - titleWidth) / 2, 180, 40, RAYWHITE);

            bool hoverW = CheckCollisionPointRec(mousePoint, btnWhite);
            DrawRectangleRec(btnWhite, hoverW ? highlightColor : lightSquare);
            DrawRectangleLinesEx(btnWhite, 3, BLACK);
            Texture2D wKing = pieceTextures[6];
            DrawTexturePro(wKing, { 0, 0, (float)wKing.width, (float)wKing.height }, { 210, 335, 120, 120 }, { 0, 0 }, 0.0f, WHITE);
            DrawText("WHITE", 225, 470, 28, BLACK);

            bool hoverB = CheckCollisionPointRec(mousePoint, btnBlack);
            DrawRectangleRec(btnBlack, hoverB ? highlightColor : darkSquare);
            DrawRectangleLinesEx(btnBlack, 3, RAYWHITE);
            Texture2D bKing = pieceTextures[-6];
            DrawTexturePro(bKing, { 0, 0, (float)bKing.width, (float)bKing.height }, { 470, 335, 120, 120 }, { 0, 0 }, 0.0f, WHITE);
            DrawText("BLACK", 485, 470, 28, hoverB ? BLACK : RAYWHITE);

            EndDrawing();
            continue;
        }

        // Allow pressing 'R' when game is over to return to menu
        if(gameOver && IsKeyPressed(KEY_R)){
            currentState = MENU;
            continue;
        }

        // --- 1A. AI ENGINE TURN (Waits 2 seconds after player's move) ---
        bool isBotTurn = (!gameOver && promotionSquare == -1 && (game.whiteTurn != playAsWhite));
        if(isBotTurn && (GetTime() - botTurnStartTime >= BOT_DELAY)){
            EngineMove aiMove = findBestMove(game, SEARCH_DEPTH);
            if (aiMove.fromSq != -1) {
                lastMoveFrom = aiMove.fromSq;
                lastMoveTo   = aiMove.toSq;
                applyEngineMove(game, aiMove);
                checkGameEnd();
            }
        }

        // --- 1B. HUMAN MOUSE INPUT ---
        if(!gameOver && (game.whiteTurn == playAsWhite) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            int mouseX = GetMouseX();
            int mouseY = GetMouseY();

            if(promotionSquare != -1){
                for (int i = 0; i < 4; i++) {
                    int bx = menuStartX + i * TILE_SIZE;
                    int by = menuStartY;

                    if (mouseX >= bx && mouseX < bx + TILE_SIZE &&
                        mouseY >= by && mouseY < by + TILE_SIZE) {
                        
                        int chosenPiece = game.whiteTurn ? promoTypes[i] : -promoTypes[i];
                        setPieceAtSquare(game, promotionSquare, chosenPiece);
                        
                        promotionSquare = -1;
                        game.whiteTurn = !game.whiteTurn;
                        checkGameEnd();
                        botTurnStartTime = GetTime(); 
                        break;
                    }
                }
            } 
            else{
                int mouseCol = mouseX / TILE_SIZE;
                int mouseRow = mouseY / TILE_SIZE;

                if(mouseRow >= 0 && mouseRow < 8 && mouseCol >= 0 && mouseCol < 8){
                    int logicalRow = playAsWhite ? mouseRow : (7 - mouseRow);
                    int logicalCol = playAsWhite ? mouseCol : (7 - mouseCol);
                    int clickedSq = logicalRow * 8 + logicalCol;

                    int clickedPiece = getPieceAtSquare(game, clickedSq);
                    bool clickedOwnPiece = (clickedPiece != 0) && ((clickedPiece > 0) == game.whiteTurn);

                    if(selectedSquare == -1){
                        if (clickedOwnPiece){
                            selectedSquare = clickedSq;
                            currentLegalMoves = getLegalMoves(game, selectedSquare);
                        }
                    } 
                    else{
                        if(clickedOwnPiece){
                            selectedSquare = clickedSq;
                            currentLegalMoves = getLegalMoves(game, selectedSquare);
                        } 
                        else if(getBit(currentLegalMoves, clickedSq)){
                            int movingPiece = getPieceAtSquare(game, selectedSquare);
                            lastMoveFrom = selectedSquare;
                            lastMoveTo   = clickedSq;
                            movePiece(game, selectedSquare, clickedSq);
                            selectedSquare = -1;
                            currentLegalMoves = 0ULL;

                            if((movingPiece == 1 && clickedSq < 8) || (movingPiece == -1 && clickedSq >= 56)){
                                promotionSquare = clickedSq;
                            } 
                            else{
                                game.whiteTurn = !game.whiteTurn;
                                checkGameEnd();
                                botTurnStartTime = GetTime();
                            }
                        } 
                        else{
                            selectedSquare = -1;
                            currentLegalMoves = 0ULL;
                        }
                    }
                }
            }
        }

        int checkedKingSq = -1;
        if (isInCheck(game, true))  checkedKingSq = getKingSquare(game, true);
        if (isInCheck(game, false)) checkedKingSq = getKingSquare(game, false);

        // --- 2. RENDER BOARD, HIGHLIGHTS & PIECES ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for(int row = 0; row < BOARD_SIZE; row++){
            for (int col = 0; col < BOARD_SIZE; col++) {
                int logicalRow = playAsWhite ? row : (7 - row);
                int logicalCol = playAsWhite ? col : (7 - col);
                int sq = logicalRow * 8 + logicalCol;

                bool isLight = (logicalRow + logicalCol) % 2 == 0;
                Color sqColor = isLight ? lightSquare : darkSquare;

                if(sq == checkedKingSq){
                    sqColor = checkColor;
                } 
                else if(sq == selectedSquare || sq == promotionSquare){
                    sqColor = highlightColor;
                } 
                else if(sq == lastMoveFrom || sq == lastMoveTo){
                    sqColor = lastMoveColor;
                }

                DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, sqColor);

                int piece = getPieceAtSquare(game, sq);
                if(piece != 0){
                    Texture2D tex = pieceTextures[piece];
                    Rectangle sourceRect = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                    Rectangle destRect = {
                        (float)(col * TILE_SIZE),
                        (float)(row * TILE_SIZE),
                        (float)TILE_SIZE,
                        (float)TILE_SIZE
                    };
                    DrawTexturePro(tex, sourceRect, destRect, { 0.0f, 0.0f }, 0.0f, WHITE);
                }

                if(getBit(currentLegalMoves, sq)){
                    int centerX = col * TILE_SIZE + TILE_SIZE / 2;
                    int centerY = row * TILE_SIZE + TILE_SIZE / 2;
                    if(piece == 0){
                        DrawCircle(centerX, centerY, TILE_SIZE * 0.16f, moveIndicator);
                    } 
                    else{
                        DrawRing({(float)centerX, (float)centerY}, TILE_SIZE * 0.38f, TILE_SIZE * 0.46f, 0, 360, 32, moveIndicator);
                    }
                }
            }
        }

        // --- 3. RENDER PROMOTION POPUP OVERLAY ---
        if(promotionSquare != -1){
            DrawRectangle(0, 0, BOARD_SIZE * TILE_SIZE, BOARD_SIZE * TILE_SIZE, { 0, 0, 0, 150 });
            DrawRectangle(menuStartX - 10, menuStartY - 50, 4 * TILE_SIZE + 20, TILE_SIZE + 60, { 45, 45, 45, 255 });
            DrawText("Promote Pawn To:", menuStartX + 65, menuStartY - 38, 28, RAYWHITE);

            int mouseX = GetMouseX();
            int mouseY = GetMouseY();

            for(int i=0; i<4; i++){
                int bx = menuStartX + i * TILE_SIZE;
                int by = menuStartY;

                bool hovered = (mouseX >= bx && mouseX < bx + TILE_SIZE && mouseY >= by && mouseY < by + TILE_SIZE);

                Color btnColor = hovered ? highlightColor : lightSquare;
                DrawRectangle(bx, by, TILE_SIZE, TILE_SIZE, btnColor);
                DrawRectangleLines(bx, by, TILE_SIZE, TILE_SIZE, BLACK);

                int optionPiece = game.whiteTurn ? promoTypes[i] : -promoTypes[i];
                Texture2D tex = pieceTextures[optionPiece];
                Rectangle sourceRect = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                Rectangle destRect   = { (float)bx, (float)by, (float)TILE_SIZE, (float)TILE_SIZE };
                DrawTexturePro(tex, sourceRect, destRect, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
        }

        // --- 4. RENDER GAME OVER BANNER ---
        if(gameOver){
            DrawRectangle(0, BOARD_SIZE * TILE_SIZE / 2 - 90, BOARD_SIZE * TILE_SIZE, 180, { 20, 20, 20, 230 });
            DrawRectangleLines(0, BOARD_SIZE * TILE_SIZE / 2 - 90, BOARD_SIZE * TILE_SIZE, 180, RAYWHITE);

            int textWidth = MeasureText(winMessage.c_str(), 40);
            DrawText(winMessage.c_str(), (BOARD_SIZE * TILE_SIZE - textWidth) / 2, BOARD_SIZE * TILE_SIZE / 2 - 45, 40, RAYWHITE);

            const char* subText = "Press 'R' to Return to Menu";
            int subWidth = MeasureText(subText, 24);
            DrawText(subText, (BOARD_SIZE * TILE_SIZE - subWidth) / 2, BOARD_SIZE * TILE_SIZE / 2 + 25, 24, highlightColor);
        }
        EndDrawing();
    }

    for(auto &pair : pieceTextures){
        UnloadTexture(pair.second);
    }

    CloseWindow();
    return 0;
}
