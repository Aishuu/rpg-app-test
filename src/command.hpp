#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "util.hpp"
#include "network.hpp"

#define COMMAND_TYPE_SIZE       2
#define LOG         0
#define BROAD       1
#define USR_DCN     2
#define USR_CON     3
#define GAME        4
#define USR_NAME    5
#define USR_ID      6

#define COMMAND_TYPE(c) (*((uint16_t *) c))
#define SET_COMMAND_TYPE(c, t) (*((uint16_t *) c)) = t

typedef uint16_t CMD_TYPE;

#define COMMAND_LENGTH_SIZE     2

#define COMMAND_LENGTH(c) (*(((uint16_t *) c) + 1))
#define SET_COMMAND_LENGTH(c, t) (*(((uint16_t *) c) + 1)) = t

typedef uint16_t CMD_LENGTH;

#define COMMAND_PAYLOAD_SIZE ((1 << (8*COMMAND_LENGTH_SIZE)) - 1)

#define COMMAND_PAYLOAD(c) ((char *) (((uint16_t *) c) + 2))

class Game;

enum CmdTarget {GM, PLAYER, BOTH};

class Command {
public:
    virtual ~Command () {}

    virtual CmdTarget target () = 0;
    virtual void execute (Game * game) = 0;
    virtual void toString (char * buffer) = 0;

    static Command * commandFromString (SOCKET from, const char * str);
};

class LogCommand : public Command {
private:
    char * _message;

public:
    LogCommand (const char * message);
    ~LogCommand ();

    CmdTarget target () { return BOTH; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class BroadcastCommand : public Command {
private:
    Command * _command;

public:
    BroadcastCommand (Command * command) : _command (command) {}
    ~BroadcastCommand ();

    CmdTarget target () { return GM; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class UserConnectivityCommand : public Command {
private:
    bool        _connected;
    uint16_t    _publicID;
    char        _name[MAX_NAME_SIZE+1];

public:
    UserConnectivityCommand (bool connected, uint16_t publicID, const char * name);
    UserConnectivityCommand (bool connected, uint16_t publicID);

    CmdTarget target () { return PLAYER; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class GameSetupCommand : public Command {
private:
    uint32_t    _privateID;
    uint16_t    _publicID;
    char        _fieldTest;

public:
    GameSetupCommand (uint32_t privateID, uint16_t publicID, char fieldTest);

    uint32_t privateID () { return _privateID; }
    uint16_t publicID () { return _publicID; }

    CmdTarget target () { return PLAYER; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class UserNameCommand : public Command {
private:
    char    _name [MAX_NAME_SIZE+1];
    SOCKET  _shadowSocket;

public:
    UserNameCommand (SOCKET shadowSocket, const char * name);

    CmdTarget target () { return GM; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class UserIDCommand : public Command {
private:
    SOCKET      _shadowSocket;
    uint32_t    _privateID;

public:
    UserIDCommand (SOCKET shadowSocket, uint32_t privateID) : _shadowSocket (shadowSocket), _privateID (privateID) {}

    CmdTarget target () { return GM; }
    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

#endif // COMMAND_HPP
