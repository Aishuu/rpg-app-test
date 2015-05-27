#include "CliGUI.hpp"
#include "game.hpp"

void CliGUI::mainLoop () {
    char buffer [200];

    if (this->_game->isGM ())
        return;
    PGame * ggame = (PGame *) this->_game;

    for (;;) {
        scanf ("%s", buffer);
        if (ggame->isDisconnected ()) {
            ggame->sendName (buffer);
        } else if (ggame->isConnected ()) {
            LogCommand * c = new LogCommand (buffer);
            BroadcastCommand cc (c);
            ggame->sendCommandToServer (&cc);
        }
    }
}
