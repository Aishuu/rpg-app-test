#ifndef GAME_H
#define GAME_H

#include "define.h"

typedef struct {
    enum piece_type  p_type;
    enum piece_color p_color;
} Piece;

typedef struct {
    Piece pieces[8][8];
    enum piece_color tour;
    struct {uint8_t x; uint8_t y;} passing;
    uint8_t N_rock;
    uint8_t B_rock;
} * Game;

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
    uint8_t rock;
    uint8_t passant;
} Coup;

#define PAT     1
#define MAT     2

Game game_new_game ();
void game_free_game (Game g);
int game_coup (Game game);

#endif // GAME_H
