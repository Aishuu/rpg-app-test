#include <stdlib.h>

#include "game.h"

Game game_new_game () {
    Game g = (Game) malloc (sizeof(*g));
    if (g == NULL)
        return NULL;

    int i, j;
    for (i=0; i<8; i++)
        for (j=0; j<8; j++) {
            g->pieces[i][j].p_type = RIEN;
            g->pieces[i][j].p_color = (j<4? BLANC: NOIR);
        }

    for (i=0; i<8; i++)
        g->pieces[i][1].p_type = g->pieces[i][6].p_type = PION;

    g->pieces[A1].p_type = g->pieces[H1].p_type = g->pieces[A8].p_type = g->pieces[H8].p_type = TOUR;
    g->pieces[B1].p_type = g->pieces[G1].p_type = g->pieces[B8].p_type = g->pieces[G8].p_type = CAVALIER;
    g->pieces[C1].p_type = g->pieces[F1].p_type = g->pieces[C8].p_type = g->pieces[F8].p_type = FOU;
    g->pieces[D1].p_type = g->pieces[D8].p_type = DAME;
    g->pieces[E1].p_type = g->pieces[E8].p_type = ROI;

    g->tour = BLANC;
    g->passing.x = 8;
    g->N_rock = 1;
    g->B_rock = 1;

    return g;
}

void game_free_game (Game g) {
    free(g);
}

#define OTHER_COLOR (c) (c == BLANC ? NOIR: BLANC)

char game_is_chess (Game game, piece_color color) {
}

int game_possible_coup (Game game, Coup *coups) {
    int c = 0;
    int i,j;
    piece_type backup_type;

    piece_color color = game->tour;
    piece_color o_color = OTHER_COLOR (color);

    for (i=0; i<8; i++)
        for (j=0; j<8; j++)
            if (game->pieces[i][j].p_color == color) {
                backup_type = game->pieces[i][j].p_type;
                switch (game->pieces[i][j].p_type) {
                    case PION:
                        if (color == BLANC) {
                            if (j < 7 && game->pieces[i][j+1].p_type == RIEN) {
                                game->pieces[i][j].p_type = RIEN;
                                if (!game_is_chess (game, o_color)) {
                                    coups[c].x1 = i;
                                    coups[c].y1 = j;
                                    coups[c].x2 = i;
                                    coups[c].y2 = j+1;
                                    c++;
                                }
                                game->pieces[i][j].p_type = backup_type;
                                if (j == 1 && game->pieces[i][3].p_type == RIEN) {
                                    game->pieces[i][j].p_type = RIEN;
                                    if (!game_is_chess (game, o_color)) {
                                        coups[c].x1 = i;
                                        coups[c].y1 = 1;
                                        coups[c].x2 = i;
                                        coups[c].y2 = 3;
                                        c++;
                                    }
                                    game->pieces[i][j].p_type = backup_type;

                            }
                        }
                }
            }
    return c;
}

int game_coup (Game game) {
    Coup possibles[448];
    int n = game_possible_coup (game, possibles);
    if (n == 0)
        return 1;

    n = rand() % n;

    // TODO: rock & passant
    game->pieces[possibles[n].x2][possibles[n].y2].p_type = game->pieces[possibles[n].x1][possibles[n].y1];
    game->pieces[possibles[n].x2][possibles[n].y2].p_color = game->pieces[possibles[n].x1][possibles[n].y1];
    game->pieces[possibles[n].x1][possibles[n].y1].p_type = RIEN;

    game->tour = OTHER_COLOR (game->tour);
    return 0;
}
