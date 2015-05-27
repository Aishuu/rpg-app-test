#include "networkAdapter.hpp"
#include "game.hpp"
#include "exceptions.hpp"

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

    Command * c = Command::commandFromString (s, buffer);
    // FIXME: raise error instead
    if (c == NULL)
        throw bad_command_error (true, 0);

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
    }
}

void GMNetworkAdapter::monitorCommands (Player * players) {
    fd_set rdfs;
    short i;
    
    for (;;) {
        FD_ZERO (&rdfs);
        
        FD_SET (this->_serverSocket, &rdfs);
        for (i=0; i<MAX_PLAYERS; i++)
            if (players[i].isConnected())
                FD_SET (players[i].getSock(), &rdfs);
        for (i=0; i<MAX_PLAYERS; i++)
            if (this->_shadowPlayers[i].isConnected())
                FD_SET (this->_shadowPlayers[i].sock(), &rdfs);

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

            for (i=0; i<MAX_PLAYERS; i++)
                if (!this->_shadowPlayers[i].isConnected ()) {
                    this->_shadowPlayers[i].waitName (csock);
                    break;
                }
            if (i == MAX_PLAYERS)
                error ("Couldn't accept new player.");

        } else {
            for (i=0; i < MAX_PLAYERS; i++) {
                SOCKET s = 0;

                if (players[i].isConnected() && (s = players[i].getSock()) && FD_ISSET (s, &rdfs)) {
                    try {
                        Command * c = receiveCommand (s);

                        if (c == NULL) {
                            players[i].disconnect ();
                            this->_game->broadcastCommand (new UserConnectivityCommand (false, players[i].publicID ()));
                            debug (DBG_BASE, "User %s has been disconnected.", players[i].name ());
                        } else
                                this->_game->pushCommand (c);

                    } catch (const bad_command_error & e) {
                        warning (e.what ());
                    }

                    break;
                }

                if (this->_shadowPlayers[i].isConnected() && (s = this->_shadowPlayers[i].sock()) && FD_ISSET (s, &rdfs)) {
                    try {
                        Command * c = receiveCommand (s);

                        if (c == NULL)
                            this->_shadowPlayers[i].clear ();
                        else
                            this->_game->pushCommand (c);

                    } catch (const bad_command_error & e) {
                        warning (e.what ());
                    }

                    break;
                }
            }
        }
    }
}

void GMNetworkAdapter::sendCommand (Player * p, Command * c) {
    if (p->isConnected ())
        this->sendCommandOnSock (c, p->getSock ());
    else
        warning ("Couldn't send command since player is disconnected.");
}

void GMNetworkAdapter::broadcastCommandExcept (Command * c, Player * players, Player * p) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        if (players[i].isConnected () && &(players[i]) != p)
            this->sendCommandOnSock (c, players[i].getSock ());
}

ShadowPlayer * GMNetworkAdapter::getShadowPlayerFromSock (SOCKET sock) {
    short i;
    for (i=0; i<MAX_PLAYERS; i++)
        if (this->_shadowPlayers[i].sock () == sock)
            return &(this->_shadowPlayers[i]);

    return NULL;
}

PNetworkAdapter::PNetworkAdapter (Game * game, const char * serverName, uint16_t serverPort) : NetworkAdapter (game), _serverName (serverName), _serverPort (serverPort), _connected (false) {
    this->connectToServer ();
}

PNetworkAdapter::~PNetworkAdapter () {
    this->closeSocket ();
}

void PNetworkAdapter::connectToServer () {
    // TODO: change errors to exceptions
    struct hostent *hostinfo = NULL;
    SOCKADDR_IN sin = { 0 };

    this->_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(this->_sock == INVALID_SOCKET)
        throw network_error ("Error when invoking socket()");

    hostinfo = gethostbyname(this->_serverName);
    if (hostinfo == NULL)
        throw network_error ("Unknown host");

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_port = htons(this->_serverPort);
    sin.sin_family = AF_INET;

    if (connect(this->_sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
        throw network_error ("Error when invoking connect()");

    this->_connected = true;
}

void PNetworkAdapter::closeSocket () {
    if (this->_connected) {
        closesocket (this->_sock);
        this->_connected = false;
    }
}

void PNetworkAdapter::monitorCommands () {
    PGame * game = (PGame *) this->_game;
    for (;;) {
        try {
            Command * c = this->receiveCommand (this->_sock);

            if (c != NULL)
                game->pushCommand (c);
            else {
                warning ("Connection to server lost.");
                game->reconnecting ();
                this->_connected = false;
                int i = 0;
                while (! this->_connected) {
                    usleep (SERVER_RECONNECTION_DELAY);
                    // TODO: c'etait pour se marrer
                    i += SERVER_RECONNECTION_DELAY;
                    printf ("\r");
                    int j;
                    for (j=0; j<(i/1000000)%5; j++)
                        printf (" ");
                    printf (".     ");
                    fflush (stdout);
                    try {
                        this->connectToServer ();
                    } catch (const network_error & e) {
                        //warning (e.what ());
                    }
                }
                printf ("\r");
                UserNameCommand c (this->_sock, game->playerMe ()->name ());
                this->sendCommandToServer (&c);
                UserIDCommand cc (0, game->playerMe ()->privateID ());
                this->sendCommandToServer (&cc);
                game->reconnected ();
            }

        } catch (const bad_command_error & e) {
            warning (e.what ());
        }
    }
}

void PNetworkAdapter::sendCommandToServer (Command * c) {
    this->sendCommandOnSock (c, this->_sock);
}
