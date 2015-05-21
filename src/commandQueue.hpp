#ifndef COMMAND_QUEUE_HPP
#define COMMAND_QUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

#include "util.hpp"
#include "command.hpp"

class CommandQueue {
private:
    std::mutex              _mutex;
    std::condition_variable _condition;
    std::queue<Command *>   _queue;

public:
    void pushCommand (Command * c);
    Command * getCommand ();
};

#endif // COMMAND_QUEUE_HPP
