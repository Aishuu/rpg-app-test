#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "util.hpp"
#include "command.hpp"

#include <iostream>
#include <sstream>
#include <exception>

class bad_command_error : public std::exception {
private:
    std::string     _msg;

public:
    bad_command_error (bool typeRecognized, CMD_TYPE type);

    virtual ~bad_command_error () throw() {}

    virtual const char * what () const throw ();
};

class network_error : public std::exception {
private:
    std::string     _msg;

public:
    network_error (std::string msg);

    virtual ~network_error () throw() {}

    virtual const char * what () const throw ();
};

#endif // EXCEPTION_HPP
