#include <string.h>

#include "command.hpp"
#include "game.hpp"

Command * Command::commandFromString (SOCKET from, const char * str) {
    CMD_TYPE t = COMMAND_TYPE (str);
    CMD_LENGTH l = COMMAND_LENGTH (str);
    Command * c = NULL;
    switch (t) {
        case LOG:
            if (l > 0 && l < COMMAND_PAYLOAD_SIZE - 1) {
                COMMAND_PAYLOAD (str)[l] = 0;
                c = new LogCommand (COMMAND_PAYLOAD (str));
            }
            else
                warning ("Bad syntax for LOG.");
            break;
        case BROAD:
        {
            if (l >= COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE) {
                Command * innerC = Command::commandFromString (from, COMMAND_PAYLOAD (str));
                if (c != NULL)
                    c = new BroadcastCommand (innerC);
            }
            if (c == NULL)
                warning ("Bad syntax for BROAD.");
        }
            break;
        case USR_DCN:
            if (l == 2) {
                uint16_t publicID = *((uint16_t *) COMMAND_PAYLOAD (str));
                if (publicID < MAX_PLAYERS)
                    c = new UserConnectivityCommand (false, publicID);
            }
            if (c == NULL)
                warning ("Bad syntax for USR_DCN.");
            break;
        case USR_CON:
            if (l > 2 && l <= MAX_NAME_SIZE + 2) {
                // TODO: change everything to user network bit order
                uint16_t publicID = *((uint16_t *) COMMAND_PAYLOAD (str));
                if (publicID < MAX_PLAYERS)
                    c = new UserConnectivityCommand (true, publicID, COMMAND_PAYLOAD (str) + 2);
            }
            if (c == NULL)
                warning ("Bad syntax for USR_CON.");
            break;
        case GAME:
            if (l > 6) {
                uint32_t privateID = *((uint32_t *) COMMAND_PAYLOAD (str));
                uint32_t publicID = *((uint16_t *) COMMAND_PAYLOAD (str) + 4);
                c = new GameSetupCommand (privateID, publicID, COMMAND_PAYLOAD (str)+6);
            }
            else
                warning ("Bad syntax for USR_CON.");
            break;
        case USR_NAME:
            if (l > 0 && l <= MAX_NAME_SIZE) {
                COMMAND_PAYLOAD (str)[l] = 0;
                int i;
                for (i=0; i<l; i++)
                    if (! VALID_NAME_CHAR (COMMAND_PAYLOAD (str)[i]))
                        break;
                if (i == l)
                    c = new UserNameCommand (from, COMMAND_PAYLOAD (str));
            }
            if (c == NULL)
                warning ("Invalid name for USR_NAME.");
            break;
        case USR_ID:
            if (l == 4) {
                uint32_t privateID = *((uint32_t *) COMMAND_PAYLOAD (str));
                c = new UserIDCommand (from, privateID);
            } else
                warning ("Bad syntax for USR_ID.");
            break;
        default:
            warning ("command %d not recognized.", t);
    }
    return c;
}

LogCommand::LogCommand (const char * message) {
    this->_id = id;
    size_t l = strlen (message);
    this->_message = (char *) malloc (sizeof (char) * (l+1));
    strncpy (this->_message, message, l);
    this->_message [l] = 0;
}

LogCommand::~LogCommand () {
    if (this->_message)
        free (this->_message);
}

void LogCommand::execute (Game * game) {
    debug ("%s", this->_message);
}

void LogCommand::toString (char * buffer) {
    SET_COMMAND_TYPE (buffer, LOG);

    size_t l = strlen (this->_message);
    if (l > COMMAND_PAYLOAD_SIZE - 1)
        l = COMMAND_PAYLOAD_SIZE - 1;

    strncpy (COMMAND_PAYLOAD(buffer), this->_message, l);
    COMMAND_PAYLOAD(buffer)[l] = 0;

    SET_COMMAND_LENGTH (buffer, l);
}

BroadcastCommand::~BroadcastCommand () {
    if (this->_command)
        free (this->_command);
}

void BroadcastCommand::execute (Game * game) {
    game->broadcastCommand (this->_command);
}

void BroadcastCommand::toString (char * buffer) {
    char buffer_tmp[COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE + COMMAND_PAYLOAD_SIZE];

    this->_command->toString (buffer_tmp);
    CMD_LENGTH l = COMMAND_LENGTH (buffer_tmp);

    if (l + COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE > COMMAND_PAYLOAD_SIZE)
        error ("The command is to big to be broadcast.");

    SET_COMMAND_TYPE (buffer, BROAD);

    size_t l = strlen (this->_message);
    if (l > COMMAND_PAYLOAD_SIZE - 1)
        l = COMMAND_PAYLOAD_SIZE - 1;

    strncpy (COMMAND_PAYLOAD(buffer), buffer_tmp, l + COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE);

    SET_COMMAND_LENGTH (buffer, l + COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE);
}

UserConnectivityCommand::UserConnectivityCommand (bool connected, uint16_t publicID, const char * name) : _connected (connected), _publicID (publicID) {
    strncpy (this->_name, name, MAX_NAME_SIZE);
    this->_name [MAX_NAME_SIZE] = 0;
}

UserConnectivityCommand::UserConnectivityCommand (bool connected, uint16_t publicID) : UserConnectivityCommand (connected, publicID, "") {}

void UserConnectivityCommand::execute (Game * game) {
    if (this->_connected)
        game->newUser (this->_publicID, this->_name);
    else
        game->userDC (this->_publicID);
}

void UserConnectivityCommand::toString (char * buffer) {
    *((uint16_t *) COMMAND_PAYLOAD (buffer)) = this->_publicID;
    SET_COMMAND_LENGTH (buffer, 2);

    if (this->_connected) {
        SET_COMMAND_TYPE (buffer, USR_CON);
        strncpy (COMMAND_PAYLOAD (buffer) + 2, this->_name, MAX_NAME_SIZE);
        SET_COMMAND_LENGTH (buffer, 2 + strlen (this->_name));
    }
    else
        SET_COMMAND_TYPE (buffer, USR_DCN);
}

GameSetupCommand::GameSetupCommand (uint32_t privateID, uint16_t publicID, const char * str) : _privateID (privateID), _publicID (publicID) {
    // TODO init fields
}

void GameSetupCommand::execute (Game * game) {
    game->loadState (this);
}

void GameSetupCommand::toString (char * buffer) {
    // TODO: set this according to fields' values
    SET_COMMAND_TYPE (buffer, GAME);
    SET_COMMAND_LENGTH (buffer, 0);
}
