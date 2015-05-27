#ifndef GUI_HPP
#define GUI_HPP

#include "util.hpp"

class Game;

class GUI {
protected:
    Game * _game;

public:
    GUI (Game * game) : _game (game) {}
    virtual ~GUI () {};

    virtual void mainLoop () = 0;
};

#endif // GUI_HPP
