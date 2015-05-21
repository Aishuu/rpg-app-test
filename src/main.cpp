#include <thread>

#include "util.hpp"
#include "game.hpp"

void testCommand (Game * game) {
    int i;
    for (i=0; i<100; i++) {
        BroadcastTestCommand * c = new BroadcastTestCommand ("test");
        game->pushCommand (c);
        sleep (2);
    }
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
        debug (DBG_BASE, "Client started.");
        game->start ();
        delete game;

        return 0;
    }

    if (argc == 2)
        game = new GMGame (atoi (argv[1]));
    else
        game = new GMGame ();

    debug (DBG_BASE, "Server started on port %d.", ((GMNetworkAdapter *) game->nwAdapter())->port());

    std::thread test (testCommand, game);
    game->start ();
    delete game;

    return 0;
}
