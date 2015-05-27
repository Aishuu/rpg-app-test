#include <thread>
#include <string.h>

#include "game.hpp"
#include "CliGUI.hpp"

Game::Game (bool isGM) : _isGM (isGM) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        this->_players[i].setPublicID (i);
}

Game::~Game () {
    if (this->_nwAdapter)
        delete this->_nwAdapter;
    if (this->_gui)
        delete this->_gui;
}

IndexedObj * Game::getIndexedObj (uint32_t id) {
    std::map<uint32_t, IndexedObj *>::iterator i = this->_objects.find(id);
    if (i != this->_objects.end())
        return i->second;
    return NULL;
}

void Game::createObject (uint32_t id, IndexedObj * obj) {
    this->_objects[id] = obj;
}

void Game::mainLoop () {
    for (;;) {
        Command * c = this->_commandQueue.getCommand ();

        CmdTarget t = c->target ();
        if (t == BOTH || (t == GM && this->_isGM) || (t == PLAYER && !this->_isGM))
            c->execute (this);

        delete c;
        // TODO: notify GUI
    }
}

void Game::pushCommand (Command * c) {
    this->_commandQueue.pushCommand (c);
}

void Game::loadState (GameSetupCommand * command) {
    // TODO: read fields from command and update
    debug (DBG_ALL, "Game loaded.");
}

void Game::newPlayer (uint16_t publicID, const char * name) {
    if (publicID < MAX_PLAYERS && !this->_players[publicID].isPresent ()) {
        this->_players[publicID].connect (name, 0);
        debug (DBG_ALL, "User %s is now connected.", name);
    }
}

void Game::playerDC (uint16_t publicID) {
    if (publicID < MAX_PLAYERS && this->_players[publicID].isConnected ()) {
        this->_players[publicID].disconnect ();
        debug (DBG_ALL, "User %s has been disconnected.", this->_players[publicID].name());
    }
}

Player * Game::getPlayerByName (const char * name) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        if (strncmp (name, this->_players[i].name (), MAX_NAME_SIZE) == 0)
            return &(this->_players[i]);

    return NULL;
}

GMGame::GMGame () : Game (true) {
    this->_nwAdapter = new GMNetworkAdapter (this);
    this->_gui = NULL;
}

GMGame::GMGame (uint16_t port) : Game (true) {
    this->_nwAdapter = new GMNetworkAdapter (this, port);
    this->_gui = NULL;
}

void GMGame::start () {
    std::thread nw (&GMNetworkAdapter::monitorCommands, (GMNetworkAdapter *) this->_nwAdapter, this->_players);
    // TODO: initialize GUI
    this->mainLoop ();
    nw.join ();
}

uint32_t GMGame::createObject (IndexedObj * obj) {
    Game::createObject (this->_objectId, obj);

    return this->_objectId++;
}

void GMGame::broadcastCommandExcept (Command * command, Player * p) {
    ((GMNetworkAdapter *) this->_nwAdapter)->broadcastCommandExcept (command, this->_players, p);
}

ShadowPlayer * GMGame::getShadowPlayerBySock (SOCKET sock) {
    return ((GMNetworkAdapter *) this->_nwAdapter)->getShadowPlayerFromSock (sock);
}

Player * GMGame::createPlayerFromShadow (ShadowPlayer * shadowPlayer, const char * name) {
    uint16_t publicID;
    for (publicID=0; publicID < MAX_PLAYERS; publicID++)
        if (!this->_players[publicID].isPresent ()) {

            uint32_t privateID;
            bool found;

            do {
                short i;
                privateID = (uint32_t) random ();
                found = (privateID == 0);
                for (i=0; i<MAX_PLAYERS; i++)
                    if (this->_players[i].isPresent () && this->_players[i].privateID () == privateID) {
                        found = true;
                        break;
                    }
            } while (found);

            this->_players[publicID].setPrivateID (privateID);
            this->_players[publicID].connect (name, shadowPlayer->sock ());
            debug (DBG_ALL, "User %s is now connected.", name);
            return &(this->_players[publicID]);
        }
    return NULL;
}

void GMGame::sendStateToPlayer (Player * player) {
    if (player->isConnected ()) {
        // TODO: Add field here
        GameSetupCommand c (player->privateID (),
                            player->publicID (),
                            this->_fieldTest);

        ((GMNetworkAdapter *) this->_nwAdapter)->sendCommand (player, &c);
    }
}

PGame::PGame (const char * serverName, uint16_t serverPort) : Game (false) {
    this->_nwAdapter = new PNetworkAdapter (this, serverName, serverPort);
    this->_status = DISCONNECTED;
    this->_gui = new CliGUI (this);
}

void PGame::start () {
    std::thread nw (&PNetworkAdapter::monitorCommands, (PNetworkAdapter *)this->_nwAdapter);
    // TODO: initialize GUI
    std::thread gui (&GUI::mainLoop, this->_gui);
    this->mainLoop ();
    nw.join ();
    gui.join ();
}

void PGame::loadState (GameSetupCommand * command) {
    if (this->_status == WAIT_GAME) {
        Game::loadState (command);
        this->_status = CONNECTED;
    }
}

void PGame::broadcastCommandExcept (Command * command, Player * p) {
    error ("Player is not allowed to broadcast a command.");
}

void PGame::sendCommandToServer (Command * command) {
    ((PNetworkAdapter *) this->_nwAdapter)->sendCommandToServer (command);
}

void PGame::createMe (uint32_t privateID, uint16_t publicID) {
    if (publicID < MAX_PLAYERS && !this->_players[publicID].isPresent ()) {
        this->_privateID = privateID;
        this->_players[publicID].connect (this->_name, 0);
        this->_playerMe = &(this->_players[publicID]);
        debug (DBG_BASE, "You are now connected as %s (%d).", this->_name, privateID);
    }
}

void PGame::sendName (const char * name) {
    if (this->_status == DISCONNECTED) {
        UserNameCommand c (0, name);
        strncpy (this->_name, name, MAX_NAME_SIZE);
        this->_name[MAX_NAME_SIZE] = 0;
        ((PNetworkAdapter *) this->_nwAdapter)->sendCommandToServer (&c);
        this->_status = WAIT_GAME;
    }
}

void PGame::reconnecting () {
    if (this->_status == CONNECTED || this->_status == WAIT_GAME)
        this->_status = RECONNECTING;
}

void PGame::reconnected () {
    if (this->_status == RECONNECTING)
        this->_status = WAIT_GAME;
}
