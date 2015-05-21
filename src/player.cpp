#include "player.hpp"

uint16_t Player::idPlayers = 0;

Player::Player () : _status (NOONE) {};

void Player::connect (uint16_t id, SOCKET sock) {
    this->_id = id;
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
    this->_id = 0;
    this->_sock = 0;
    this->_status = NOONE;
}

void Player::reconnect (SOCKET sock) {
    this->_sock = sock;
    this->_status = CONNECTED;
}

