#include <thread>

#include "util.hpp"
#include "game.hpp"

int client (Game * game) {
    debug (DBG_BASE, "Client started.");
    printf ("Please enter a name (max %d): ", MAX_NAME_SIZE);
    fflush (stdout);
    game->start ();
    delete game;

    return 0;
}

int server (Game * game) {
    debug (DBG_BASE, "Server started on port %d.", ((GMNetworkAdapter *) game->nwAdapter())->port());

    game->start ();
    delete game;

    return 0;
}

int main (int argc, char ** argv) {
    if (argc > 3) {
        fprintf (stderr, "Usage :\n");
        fprintf (stderr, "    %s [port]           server\n", argv[0]);
        fprintf (stderr, "    %s addr port        client\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    Game * game;

    if (argc == 3) {
        game = new PGame (argv[1], atoi(argv[2]));
        return client (game);
    }

    if (argc == 2)
        game = new GMGame (atoi (argv[1]));
    else
        game = new GMGame ();

    return server (game);
}
