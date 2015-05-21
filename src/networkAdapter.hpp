#ifndef NETWORK_ADAPTER_HPP
#define NETWORK_ADAPTER_HPP

#include "util.hpp"
#include "player.hpp"
#include "command.hpp"
#include "network.hpp"

class Game;

class NetworkAdapter {
protected:
    Game *      _game;

    Command * receiveCommand (SOCKET s);
    void sendCommandOnSock (Command * c, SOCKET s);

public:
    NetworkAdapter (Game * game);
    virtual ~NetworkAdapter ();
    
    virtual void monitorCommands () = 0;
};

class GMNetworkAdapter: public NetworkAdapter {
private:
    uint16_t    _port;
    SOCKET      _serverSocket;
    Player      _players [MAX_PLAYERS];
    SOCKET      _maxSocket;
    bool        _listening;

    void setupNetwork ();
    void closeSocket ();

public:
    GMNetworkAdapter (Game * game);
    GMNetworkAdapter (Game * game, uint16_t port);
    ~GMNetworkAdapter ();

    uint16_t port () { return _port; }
    virtual void monitorCommands ();
    void sendCommand (uint16_t playerID, Command * c);
    void broadcastCommand (Command * c);
};

class PNetworkAdapter: public NetworkAdapter {
private:
    const char  * _serverName;
    uint16_t    _serverPort;
    SOCKET      _sock;
    bool        _connected;

    void connectToServer ();
    void closeSocket ();

public:
    PNetworkAdapter (Game * game, const char * serverName, uint16_t serverPort);
    ~PNetworkAdapter ();

    virtual void monitorCommands ();
    void sendCommand (Command * c);
};

#endif // NETWORK_ADAPTER_HPP
