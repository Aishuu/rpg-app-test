#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "util.hpp"
#include "network.hpp"

class Player {
private:
    enum Status {CONNECTED, DISCONNECTED, NOONE};
    Status      _status;

    uint16_t    _publicID;
    uint32_t    _privateID;
    SOCKET      _sock;
    char        _name [MAX_NAME_SIZE+1];

public:
    Player ();

    uint16_t publicID () { return _publicID; }
    uint32_t privateID () { return _privateID; }
    SOCKET getSock () { return _sock; }
    char * name () { return _name; }
    bool isConnected ();
    bool isPresent ();
    void setPrivateID (uint32_t privateID) { _privateID = privateID; }
    void setPublicID (uint16_t publicID) { _publicID = publicID; }

    void connect (const char * name, SOCKET sock);
    void reconnect (SOCKET sock);
    void disconnect ();
    void reset ();
};

class ShadowPlayer {
private:
    Player          * _player;
    SOCKET          _sock;

    enum ShadowStatus {FREE, WAIT_NAME, WAIT_ID};
    ShadowStatus    _status;

public:
    ShadowPlayer () : _player (NULL), _sock (0), _status (FREE) {}

    SOCKET sock () { return _sock; }
    Player * getPlayer () { return _player; }
    bool isConnected () { return _status != FREE; }

    void waitName (SOCKET sock);
    void waitID (Player * player);
    void closeSocket ();
    void clear ();
};

#endif // PLAYER_HPP
