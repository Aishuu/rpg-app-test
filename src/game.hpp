#ifndef GAME_HPP
#define GAME_HPP

#include <map>

#include "util.hpp"
#include "indexedObj.hpp"
#include "commandQueue.hpp"
#include "networkAdapter.hpp"
#include "gui.hpp"

class Game {
private:
    CommandQueue                        _commandQueue;

protected:
    std::map<uint32_t, IndexedObj *>    _objects;
    NetworkAdapter                      * _nwAdapter;
    bool                                _isGM;
    Player                              _players [MAX_PLAYERS];
    char                                _fieldTest;
    GUI                                 * _gui;

    void mainLoop ();

public:
    Game (bool isGM);
    virtual ~Game ();

    NetworkAdapter * nwAdapter () { return _nwAdapter; }
    bool isGM () { return _isGM; }
    IndexedObj * getIndexedObj (uint32_t id);
    void createObject (uint32_t id, IndexedObj * obj);
    virtual void start () = 0;
    void pushCommand (Command * c);

    virtual void broadcastCommandExcept (Command * command, Player * p) = 0;
    virtual void broadcastCommand (Command * command) { broadcastCommandExcept (command, NULL); }
    virtual void loadState (GameSetupCommand * command);
    void newPlayer (uint16_t publicID, const char * name);
    void playerDC (uint16_t publicID);
    Player * getPlayerByName (const char * name);
};

class GMGame: public Game {
private:
    uint32_t _objectId;

public:
    GMGame ();
    GMGame (uint16_t port);
    uint32_t createObject (IndexedObj * obj);

    virtual void start ();
    void broadcastCommandExcept (Command * command, Player * p);
    ShadowPlayer * getShadowPlayerBySock (SOCKET sock);
    Player * createPlayerFromShadow (ShadowPlayer * shadowPlayer, const char * name);
    void sendStateToPlayer (Player * player);
};

class PGame: public Game {
private:
    uint32_t    _privateID;
    char        _name [MAX_NAME_SIZE+1];

    enum GameStatus {DISCONNECTED, WAIT_GAME, CONNECTED, RECONNECTING};
    GameStatus  _status;

    Player *    _playerMe;

public:
    PGame (const char * serverName, uint16_t serverPort);

    bool isDisconnected () { return this->_status == DISCONNECTED; }
    bool isWaitGame () { return this->_status == WAIT_GAME; }
    bool isConnected () { return this->_status == CONNECTED; }
    bool isReconnecting () { return this->_status == RECONNECTING; }

    Player * playerMe () { return _playerMe; }
    virtual void start ();
    virtual void loadState (GameSetupCommand * command);
    void broadcastCommandExcept (Command * command, Player * p);
    void sendCommandToServer (Command * command);
    void createMe (uint32_t privateID, uint16_t publicID);
    void sendName (const char * name);
    void reconnecting ();
    void reconnected ();
    void disconnect ();
};

#endif // GAME_HPP
