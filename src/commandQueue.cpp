#include "commandQueue.hpp"

void CommandQueue::pushCommand (Command * c) {
    {
        std::unique_lock<std::mutex> lock (this->_mutex);
        this->_queue.push(c);
    }
    this->_condition.notify_one();
}

Command * CommandQueue::getCommand () {
    std::unique_lock<std::mutex> lock (this->_mutex);
    this->_condition.wait (lock, [=]{ return !this->_queue.empty(); });
    Command * c = this->_queue.front();
    this->_queue.pop();
    return c;
}
