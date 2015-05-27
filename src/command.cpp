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
                if (innerC != NULL) {
                    c = new BroadcastCommand (innerC);
                }
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
                uint16_t publicID = *((uint16_t *) COMMAND_PAYLOAD (str) + 4);
                c = new GameSetupCommand (privateID, publicID, *(COMMAND_PAYLOAD (str)+6));
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
    debug (DBG_ALL, "%s", this->_message);
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
        delete this->_command;
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

    memcpy (COMMAND_PAYLOAD(buffer), buffer_tmp, l + COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE);

    SET_COMMAND_LENGTH (buffer, l + COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE);
}

UserConnectivityCommand::UserConnectivityCommand (bool connected, uint16_t publicID, const char * name) : _connected (connected), _publicID (publicID) {
    strncpy (this->_name, name, MAX_NAME_SIZE);
    this->_name [MAX_NAME_SIZE] = 0;
}

UserConnectivityCommand::UserConnectivityCommand (bool connected, uint16_t publicID) : UserConnectivityCommand (connected, publicID, "") {}

void UserConnectivityCommand::execute (Game * game) {
    if (this->_connected)
        game->newPlayer (this->_publicID, this->_name);
    else
        game->playerDC (this->_publicID);
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

GameSetupCommand::GameSetupCommand (uint32_t privateID, uint16_t publicID, char fieldTest) : _privateID (privateID), _publicID (publicID), _fieldTest (fieldTest) {
    // TODO init fields
}

void GameSetupCommand::execute (Game * game) {
    ((PGame *) game)->createMe (this->_privateID, this->_publicID);
    game->loadState (this);
}

void GameSetupCommand::toString (char * buffer) {
    // TODO: set this according to fields' values
    SET_COMMAND_TYPE (buffer, GAME);
    SET_COMMAND_LENGTH (buffer, 7);
    *((uint32_t *) COMMAND_PAYLOAD (buffer)) = this->_privateID;
    *((uint16_t *) COMMAND_PAYLOAD (buffer) + 4) = this->_publicID;
    *(COMMAND_PAYLOAD (buffer) + 6) = this->_fieldTest;
}

UserNameCommand::UserNameCommand (SOCKET shadowSocket, const char * name) : _shadowSocket (shadowSocket) {
    strncpy (this->_name, name, MAX_NAME_SIZE);
    this->_name [MAX_NAME_SIZE] = 0;
}

void UserNameCommand::execute (Game * game) {
    if (! game->isGM ())
        return;
    GMGame * gmgame = (GMGame *) game;

    Player * player = gmgame->getPlayerByName (this->_name);
    ShadowPlayer * shadowPlayer = gmgame->getShadowPlayerBySock (this->_shadowSocket);
    if (shadowPlayer == NULL) {
        warning ("Got a USR_NAME command from a ghost.");
        return;
    }

    if (player == NULL) {
        player = gmgame->createPlayerFromShadow (shadowPlayer, this->_name);
        if (player != NULL) {
            UserConnectivityCommand c (true, player->publicID (), player->name ());
            gmgame->broadcastCommandExcept (&c, player);
            gmgame->sendStateToPlayer (player);
        } else
            shadowPlayer->closeSocket ();

        shadowPlayer->clear ();
    } else
        shadowPlayer->waitID (player);
}

void UserNameCommand::toString (char * buffer) {
    SET_COMMAND_TYPE (buffer, USR_NAME);

    size_t l = strlen (this->_name);
    if (l > MAX_NAME_SIZE) l = MAX_NAME_SIZE;
    SET_COMMAND_LENGTH (buffer, l);

    strncpy (COMMAND_PAYLOAD (buffer), this->_name, l);
}

void UserIDCommand::execute (Game * game) {
    if (! game->isGM ())
        return;
    ShadowPlayer * shadowPlayer = ((GMGame *) game)->getShadowPlayerBySock (this->_shadowSocket);
    if (shadowPlayer == NULL)
        return;
    Player * player = shadowPlayer->getPlayer ();
    if (player == NULL)
        return;

    if (player->privateID () == this->_privateID) {
        player->reconnect (this->_shadowSocket);
        UserConnectivityCommand c (true, player->publicID (), player->name ());
        game->broadcastCommandExcept (&c, player);
    }

    shadowPlayer->clear ();
}

void UserIDCommand::toString (char * buffer) {
    SET_COMMAND_TYPE (buffer, USR_ID);
    SET_COMMAND_LENGTH (buffer, 4);
    *((uint32_t *) COMMAND_PAYLOAD (buffer)) = this->_privateID;
}
