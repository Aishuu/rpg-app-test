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
};

class GMNetworkAdapter: public NetworkAdapter {
private:
    uint16_t        _port;
    SOCKET          _serverSocket;
    SOCKET          _maxSocket;
    bool            _listening;
    ShadowPlayer    _shadowPlayers[MAX_PLAYERS];

    void setupNetwork ();
    void closeSocket ();

public:
    GMNetworkAdapter (Game * game);
    GMNetworkAdapter (Game * game, uint16_t port);
    ~GMNetworkAdapter ();

    uint16_t port () { return _port; }
    virtual void monitorCommands (Player * players);
    void sendCommand (Player * p, Command * c);
    void broadcastCommandExcept (Command * c, Player players[], Player * p);
    ShadowPlayer * getShadowPlayerFromSock (SOCKET sock);
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
    void sendCommandToServer (Command * c);
};

#endif // NETWORK_ADAPTER_HPP
