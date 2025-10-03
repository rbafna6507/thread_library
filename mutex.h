/*
 * mutex.h -- interface to the mutex class
 *
 * You may add new variables and functions to this class.
 *
 * Do not modify any of the given function declarations.
 */

#pragma once
#include <deque>
#include "tcb.h"

class mutex {
public:
    mutex();
    ~mutex();
    void lock();
    void unlock();

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    /*
     * Move constructor and move assignment operator.  Implementing these is
     * optional in Project 2.
     */
    mutex(mutex&&);
    mutex& operator=(mutex&&);

    std::deque<std::shared_ptr<tcb>> waiting_queue;
    bool busy = false;
    int owner_id;

private:
    void internal_lock();
    void internal_unlock();
    friend class cv;
};
