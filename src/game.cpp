#include <thread>

#include "game.hpp"

Game::~Game () {
    if (this->_nwAdapter)
        delete this->_nwAdapter;
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

void Game::start () {
    std::thread nw (&NetworkAdapter::monitorCommands, this->_nwAdapter);
    // TODO: initialize GUI
    this->mainLoop ();
    nw.join ();
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

void loadState (GameSetupCommand * command) {
    // TODO: read fields from command and update
}

GMGame::GMGame () : _isGM (true) {
    this->_nwAdapter = new GMNetworkAdapter (this);
}

GMGame::GMGame (uint16_t port) : _isGM (true) {
    this->_nwAdapter = new GMNetworkAdapter (this, port);
}

uint32_t GMGame::createObject (IndexedObj * obj) {
    Game::createObject (this->_objectId, obj);

    return this->_objectId++;
}

void GMGame::broadcastCommand (Command * command) {
    ((GMNetworkAdapter *) this->_nwAdapter)->broadcastCommand (command);
}

PGame::PGame (const char * serverName, uint16_t serverPort) : _isGM (false) {
    this->_nwAdapter = new PNetworkAdapter (this, serverName, serverPort);
}

void PGame::broadcastCommand (Command * command) {
    error ("Player is not allowed to broadcast a method.");
}
