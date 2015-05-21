#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "util.hpp"

#define COMMAND_TYPE_SIZE       2
#define TEST    0
#define BTST    1

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

class Command {
protected:
    uint32_t _id;

    static uint32_t currentId;

public:
    virtual ~Command () {}

    virtual void execute (Game * game) = 0;
    virtual void toString (char * buffer) = 0;

    static Command * commandFromString (const char * str);
};

class TestCommand : public Command {
private:
    char * _message;

public:
    TestCommand (const char * message, uint32_t id);
    TestCommand (const char * message);
    ~TestCommand ();

    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

class BroadcastTestCommand : public Command {
private:
    char * _message;

public:
    BroadcastTestCommand (const char * message, uint32_t id);
    BroadcastTestCommand (const char * message);
    ~BroadcastTestCommand ();

    virtual void execute (Game * game);
    virtual void toString (char * buffer);
};

#endif // COMMAND_HPP
