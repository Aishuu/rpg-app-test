#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "util.hpp"
#include "network.hpp"

enum Status {CONNECTED, DISCONNECTED, NOONE};

class Player {
private:
    Status      _status;
    uint16_t    _id;
    SOCKET      _sock;

    static uint16_t idPlayers;

public:
    Player ();

    uint16_t getId () { return _id; }
    SOCKET getSock () { return _sock; }
    bool isConnected () { return _status == CONNECTED; }
    bool isPresent () { return _status != NOONE; }

    void connect (uint16_t id, SOCKET sock);
    void disconnect ();
    void reset ();
    void reconnect (SOCKET sock);

    static uint16_t newId () { return idPlayers++; }
};

#endif // PLAYER_HPP
