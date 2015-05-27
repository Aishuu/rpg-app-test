#include <string.h>

#include "player.hpp"

Player::Player () : _status (NOONE) {};

bool Player::isConnected () { return _status == CONNECTED; }
bool Player::isPresent () { return _status != NOONE; }

void Player::connect (const char * name, SOCKET sock) {
    strncpy (this->_name, name, MAX_NAME_SIZE);
    this->_name[MAX_NAME_SIZE] = 0;
    this->_sock = sock;
    this->_status = CONNECTED;
}

void Player::disconnect () {
    if (this->_status == CONNECTED)
        closesocket (this->_sock);
    this->_status = DISCONNECTED;
}

void Player::reset () {
    if (this->_status == CONNECTED)
        closesocket (this->_sock);
    this->_privateID = 0;
    this->_publicID = 0;
    this->_sock = 0;
    this->_status = NOONE;
}

void Player::reconnect (SOCKET sock) {
    this->_sock = sock;
    this->_status = CONNECTED;
}

void ShadowPlayer::waitName (SOCKET sock) {
    if (this->_status == FREE) {
        this->_sock = sock;
        this->_status = WAIT_NAME;
    }
}

void ShadowPlayer::waitID (Player * player) {
    if (this->_status == WAIT_NAME) {
        this->_player = player;
        this->_status = WAIT_ID;
    } else
        this->clear ();
}

void ShadowPlayer::closeSocket () {
    if (this->_status != FREE)
        closesocket (this->_sock);
}

void ShadowPlayer::clear () {
    this->_status = FREE;
    this->_sock = 0;
    this->_player = NULL;
}
