#ifndef GAME_HPP
#define GAME_HPP

#include <map>

#include "util.hpp"
#include "indexedObj.hpp"
#include "commandQueue.hpp"
#include "networkAdapter.hpp"

class Game {
private:
    CommandQueue                        _commandQueue;

protected:
    std::map<uint32_t, IndexedObj *>    _objects;
    NetworkAdapter                      * _nwAdapter;
    bool                                _isGM;

    void mainLoop ();

public:
    virtual ~Game ();

    NetworkAdapter * nwAdapter () { return _nwAdapter; }
    bool isGM () { return _isGM; }
    IndexedObj * getIndexedObj (uint32_t id);
    void createObject (uint32_t id, IndexedObj * obj);
    void start ();
    void pushCommand (Command * c);

    virtual void broadcastCommand (Command * command) = 0;
    void loadState (GameSetupCommand * command);
};

class GMGame: public Game {
private:
    uint32_t _objectId;

public:
    GMGame ();
    GMGame (uint16_t port);
    uint32_t createObject (IndexedObj * obj);

    void broadcastMessage (Command * command);
};

class PGame: public Game {
public:
    PGame (const char * serverName, uint16_t serverPort);

    void broadcastMessage (Command * command);
};

#endif // GAME_HPP
