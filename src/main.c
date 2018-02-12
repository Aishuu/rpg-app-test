#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "gui.h"

int main (int argc, char ** argv) {
    srand(time(NULL));
    Game game = game_new_game ();
    startGUI (game);
    return 0;
}
