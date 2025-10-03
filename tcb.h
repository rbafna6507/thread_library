#ifndef TCB_H
#define TCB_H

#include "thread.h"
#include <memory>
#include <ucontext.h>

struct tcb {
    // contains start of stack pointer
    std::shared_ptr<ucontext_t> ucontext_ptr;
    int thread_id;
    void* stack_pointer;
    // constructor  
    tcb();
    tcb(int thread_id, thread_startfunc_t func, uintptr_t arg);
    // destructor
    ~tcb();
};

#endif // TCB_H