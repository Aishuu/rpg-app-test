#include <string.h>

#include "command.hpp"
#include "game.hpp"

uint32_t Command::currentId = 0;

Command * Command::commandFromString (const char * str) {
    CMD_TYPE t = COMMAND_TYPE (str);
    Command * c = NULL;
    switch (t) {
        case TEST:
            c = new TestCommand (COMMAND_PAYLOAD (str));
            break;
        case BTST:
            c = new BroadcastTestCommand (COMMAND_PAYLOAD (str));
            break;
        default:
            warning ("command %d not recognized.", t);
    }
    return c;
}

TestCommand::TestCommand (const char * message, uint32_t id) {
    this->_id = id;
    size_t l = strlen (message);
    this->_message = (char *) malloc (sizeof (char) * (l+1));
    strncpy (this->_message, message, l);
    this->_message [l] = 0;
}

TestCommand::TestCommand (const char * message) : TestCommand (message, currentId++) {
}

TestCommand::~TestCommand () {
    if (this->_message)
        free (this->_message);
}

void TestCommand::execute (Game * game) {
    printf ("[%3d] %s\n", this->_id, this->_message);
}

void TestCommand::toString (char * buffer) {
    SET_COMMAND_TYPE (buffer, TEST);

    size_t l = strlen (this->_message);
    if (l > COMMAND_PAYLOAD_SIZE - 1)
        l = COMMAND_PAYLOAD_SIZE - 1;

    strncpy (COMMAND_PAYLOAD(buffer), this->_message, l);
    COMMAND_PAYLOAD(buffer)[l] = 0;

    SET_COMMAND_LENGTH (buffer, l);
}

BroadcastTestCommand::BroadcastTestCommand (const char * message, uint32_t id) {
    this->_id = id;
    size_t l = strlen (message);
    this->_message = (char *) malloc (sizeof (char) * (l+1));
    strncpy (this->_message, message, l);
    this->_message [l] = 0;
}

BroadcastTestCommand::BroadcastTestCommand (const char * message) : BroadcastTestCommand (message, currentId++) {
}

BroadcastTestCommand::~BroadcastTestCommand () {
    if (this->_message)
        free (this->_message);
}

void BroadcastTestCommand::execute (Game * game) {
    game->broadcastMessage (this->_message);
}

void BroadcastTestCommand::toString (char * buffer) {
    SET_COMMAND_TYPE (buffer, BTST);

    size_t l = strlen (this->_message);
    if (l > COMMAND_PAYLOAD_SIZE - 1)
        l = COMMAND_PAYLOAD_SIZE - 1;

    strncpy (COMMAND_PAYLOAD(buffer), this->_message, l);
    COMMAND_PAYLOAD(buffer)[l] = 0;

    SET_COMMAND_LENGTH (buffer, l);
}
