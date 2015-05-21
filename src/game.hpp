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

    void mainLoop ();

public:
    virtual ~Game ();

    NetworkAdapter * nwAdapter () { return _nwAdapter; }
    IndexedObj * getIndexedObj (uint32_t id);
    void createObject (uint32_t id, IndexedObj * obj);
    void start ();
    void pushCommand (Command * c);

    virtual void broadcastMessage (const char * message) = 0;
};

class GMGame: public Game {
private:
    uint32_t _objectId;

public:
    GMGame ();
    GMGame (uint16_t port);
    uint32_t createObject (IndexedObj * obj);

    void broadcastMessage (const char * message);
};

class PGame: public Game {
public:
    PGame (const char * serverName, uint16_t serverPort);

    void broadcastMessage (const char * message);
};

#endif // GAME_HPP
