#include <memory>
#include <ucontext.h>
#include "cpu.h"
#include "thread.h"
#include "tcb.h"

tcb::tcb(){};

tcb::tcb(int thread_id_, thread_startfunc_t func, uintptr_t arg){
    thread_id = thread_id_;
    ucontext_ptr = std::make_shared<ucontext_t>();
    ucontext_ptr->uc_stack.ss_sp = new char [STACK_SIZE];
    stack_pointer = ucontext_ptr->uc_stack.ss_sp;
    ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
    ucontext_ptr->uc_stack.ss_flags = 0;
    ucontext_ptr->uc_link = nullptr;
    makecontext(ucontext_ptr.get(), reinterpret_cast<void (*)()>(cpu::context_switcher), 2, func, arg);
}

tcb::~tcb(){
    delete[] static_cast<char*>(stack_pointer);
}
