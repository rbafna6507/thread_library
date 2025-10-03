#include "thread.h"
#include "cpu.h"
#include "raii.h"
#include <ucontext.h>
#include <cstdio>
#include <cassert>


thread::thread(thread_startfunc_t func, uintptr_t arg){
    RAII raii;
    // assert(func != nullptr);
    
    // make thread
    thread_id = cpu::make_thread(func, arg);
    // push thread to ready queue
    cpu::ready_queue.push_back(cpu::id_to_tcb[thread_id]);
    cpu::send_ipi(); // THIS CAUSES A 'interrupt enabled() called with interrupts already enabled' error, why???

};

void thread::yield(){
    RAII raii;
    // cpu::interrupt_disable();
    if (!cpu::ready_queue.empty()){
        // std::printf("ready queue not empty. yielding.\n");
        cpu::ready_queue.push_back(cpu::self()->current_context_tcb);
        swapcontext(cpu::self()->current_context_tcb->ucontext_ptr.get(), cpu::self()->os_context_ptr.get());
    }
    cpu::send_ipi();
    // cpu::interrupt_enable();
};

void thread::join(){
    RAII raii;
    // a therad trying to join itself is actually allowed just bad style + we should handle it
    // assert(thread_id != cpu::self()->current_context_tcb->thread_id);
    // cpu::interrupt_disable();
    if(cpu::id_to_tcb.find(thread_id) != cpu::id_to_tcb.end()){
        cpu::id_to_joined[thread_id].push_back(cpu::self()->current_context_tcb);
        swapcontext(cpu::self()->current_context_tcb->ucontext_ptr.get(), cpu::self()->os_context_ptr.get());
    }
    // cpu::interrupt_enable();
}


