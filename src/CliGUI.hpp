#ifndef CLI_GUI_HPP
#define CLI_GUI_HPP

#include "util.hpp"
#include "gui.hpp"

class CliGUI : public GUI {
public:
    CliGUI (Game * game) : GUI (game) {}

    virtual void mainLoop ();
};

#endif // CLI_GUI_HPP
