#include "exceptions.hpp"

bad_command_error::bad_command_error (bool typeRecognized, CMD_TYPE type) {
    std::ostringstream oss;
    if (typeRecognized)
        oss << "Syntax error for command of type " << type << ".";
    else
        oss << "Unknown command type.";
    this->_msg = oss.str();
}

const char * bad_command_error::what () const throw () {
    return this->_msg.c_str();
}

network_error::network_error (std::string msg) {
    this->_msg = msg; 
}

const char * network_error::what () const throw () {
    return this->_msg.c_str();
}
