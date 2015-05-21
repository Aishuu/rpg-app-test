#include "networkAdapter.hpp"
#include "game.hpp"

NetworkAdapter::NetworkAdapter (Game * game) : _game (game) {
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
        error("WSAStartup failed !");
#endif
}

NetworkAdapter::~NetworkAdapter () {
#ifdef WIN32
    WSACleanup();
#endif
}

Command * NetworkAdapter::receiveCommand (SOCKET s) {
    char buffer[COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE + COMMAND_PAYLOAD_SIZE];
    int n = 0, receivedB = 0;
    CMD_LENGTH l;

    while (receivedB < COMMAND_TYPE_SIZE + COMMAND_LENGTH_SIZE) {
        if((n = recv(s, buffer+receivedB, COMMAND_TYPE_SIZE + COMMAND_LENGTH_SIZE - receivedB, 0)) <= 0)
            return NULL;
        receivedB += n;
    }

    l = COMMAND_LENGTH (buffer);

    while (receivedB < COMMAND_TYPE_SIZE + COMMAND_LENGTH_SIZE + l) {
        if((n = recv(s, buffer+receivedB, COMMAND_TYPE_SIZE + COMMAND_LENGTH_SIZE + l - receivedB, 0)) <= 0)
            return NULL;
        receivedB += n;
    }

    Command * c = Command::commandFromString (buffer);

    return c;
}

void NetworkAdapter::sendCommandOnSock (Command * c, SOCKET s) {
    char buffer[COMMAND_LENGTH_SIZE + COMMAND_TYPE_SIZE + COMMAND_PAYLOAD_SIZE];
    CMD_LENGTH l;

    c->toString (buffer);
    l = COMMAND_LENGTH (buffer);

    if(send(s, buffer, COMMAND_TYPE_SIZE + COMMAND_LENGTH_SIZE + l, 0) < 0)
        error ("send failed.");
}

GMNetworkAdapter::GMNetworkAdapter (Game * game) : NetworkAdapter (game), _port (0), _listening (false) {
    this->setupNetwork ();
}

GMNetworkAdapter::GMNetworkAdapter (Game * game, uint16_t port) : NetworkAdapter (game), _port (port), _listening (false) {
    this->setupNetwork ();
}

void GMNetworkAdapter::setupNetwork () {
    this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(this->_serverSocket == INVALID_SOCKET)
        error("socket()");

    SOCKADDR_IN sin = { 0 };

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(this->_port);
    sin.sin_family = AF_INET;

    if(bind(this->_serverSocket, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
       error("bind()");

    if(listen(this->_serverSocket, MAX_PLAYERS) == SOCKET_ERROR)
       error("listen()");

   socklen_t len = sizeof(sin);
   if (getsockname(this->_serverSocket, (struct sockaddr *)&sin, &len) == -1)
       error("getsockname");
   else
       this->_port = ntohs(sin.sin_port);

    this->_maxSocket = this->_serverSocket;
}

GMNetworkAdapter::~GMNetworkAdapter () {
    this->closeSocket ();
}

void GMNetworkAdapter::closeSocket () {
    if (this->_listening) {
        closesocket (this->_serverSocket);
        this->_listening = false;
    
        int i;
        for (i=0; i<MAX_PLAYERS; i++)
            this->_players[i].reset();
    }
}

void GMNetworkAdapter::monitorCommands () {
    fd_set rdfs;
    short i;
    
    for (;;) {
        FD_ZERO (&rdfs);
        
        FD_SET (this->_serverSocket, &rdfs);
        for (i=0; i<MAX_PLAYERS; i++)
            if (this->_players[i].isConnected())
                FD_SET (this->_players[i].getSock(), &rdfs);

        if (select (this->_maxSocket + 1, &rdfs, NULL, NULL, NULL) == -1)
            error("select()");
    
        if (FD_ISSET (this->_serverSocket, &rdfs)) {
            SOCKADDR_IN csin = { 0 };
            socklen_t sinsize = sizeof (csin);
            int csock = accept (this->_serverSocket, (SOCKADDR *) &csin, &sinsize);
            if(csock == SOCKET_ERROR)
                error("accept()");

            if (csock > this->_maxSocket)
                this->_maxSocket = csock;

            // TODO: take into account reconnection
            for (i=0; i<MAX_PLAYERS; i++)
                if (!this->_players[i].isPresent ()) {
                    this->_players[i].connect (Player::newId (), csock);
                    break;
                }
            if (i == MAX_PLAYERS)
                error ("Couldn't match reconnecting player.");

            debug (DBG_BASE, "New player connected assigned ID %d.", i);

        } else {
            for (i=0; i < MAX_PLAYERS; i++) {
                SOCKET s = 0;

                if (this->_players[i].isConnected() && (s = this->_players[i].getSock()) && FD_ISSET (s, &rdfs)) {
                    Command * c = receiveCommand (s);

                    // FIXME: c may be null if command type is unknown
                    if (c == NULL) {
                        this->_players[i].disconnect ();
                        debug (DBG_BASE, "Player ID %d disconnected.", i);
                    }
                    else
                        this->_game->pushCommand (c);
                    break;
                }
            }
        }
    }
}

void GMNetworkAdapter::sendCommand (uint16_t playerID, Command * c) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        if (this->_players[i].isConnected () && this->_players[i].getId () == playerID) {
            this->sendCommandOnSock (c, this->_players[i].getSock ());
            break;
        }
    if (i == MAX_PLAYERS)
        warning ("Couldn't find player with ID %d.", playerID);
}

void GMNetworkAdapter::broadcastCommand (Command * c) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        if (this->_players[i].isConnected ())
            this->sendCommandOnSock (c, this->_players[i].getSock ());
}

PNetworkAdapter::PNetworkAdapter (Game * game, const char * serverName, uint16_t serverPort) : NetworkAdapter (game), _serverName (serverName), _serverPort (serverPort), _connected (false) {
    this->connectToServer ();
}

PNetworkAdapter::~PNetworkAdapter () {
    this->closeSocket ();
}

void PNetworkAdapter::connectToServer () {
    struct hostent *hostinfo = NULL;
    SOCKADDR_IN sin = { 0 };

    this->_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(this->_sock == INVALID_SOCKET)
        error ("Error when invoking socket()");

    hostinfo = gethostbyname(this->_serverName);
    if (hostinfo == NULL)
        error ("Unknown host %s.", this->_serverName);

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_port = htons(this->_serverPort);
    sin.sin_family = AF_INET;

    if (connect(this->_sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
        error ("Error when invoking connect()");

    this->_connected = true;
}

void PNetworkAdapter::closeSocket () {
    if (this->_connected) {
        closesocket (this->_sock);
        this->_connected = false;
    }
}

void PNetworkAdapter::monitorCommands () {
    for (;;) {
        Command * c = this->receiveCommand (this->_sock);

        if (c != NULL)
            this->_game->pushCommand (c);
        else
            error ("Connection to server lost.");
        // FIXME: else connection lost ?
    }
}

void PNetworkAdapter::sendCommand (Command * c) {
    this->sendCommandOnSock (c, this->_sock);
}
