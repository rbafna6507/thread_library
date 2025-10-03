#include "mutex.h"
#include "cpu.h"
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <cassert>
#include "raii.h"
#include <stdexcept>


// initialize waiting_queue for this variable
mutex::mutex(){}

mutex::~mutex(){}

void mutex::internal_lock(){
    // keeping test and set bc we use internal lock by itself
    while (cpu::test_and_set()) {}

    if (busy != false){
        // assert(cpu::self()->current_context_tcb != nullptr);
        // cpu code that gets what is currently running
        // push it onto the waiting queue
        //  std::printf("did not acquire lock :( \n");
         waiting_queue.push_back(cpu::self()->current_context_tcb);
         swapcontext(cpu::self()->current_context_tcb->ucontext_ptr.get(), cpu::self()->os_context_ptr.get());
    }
    else{
        busy = true;
        owner_id = cpu::self()->current_context_tcb->thread_id;
        // std::cout << "thread " << owner_id << "LOCKED" << std::endl;
    }

    // keeping guard.exchange because we use internal_lock by itself
    cpu::guard.exchange(false);
}

void mutex::internal_unlock(){
    if (cpu::self()->current_context_tcb->thread_id == owner_id){
        while (cpu::test_and_set()){}

        busy = false;
        // std::printf("let go of lock \n");
        // std::cout << "thread " << owner_id << "UNLOCKED" << std::endl;


        if (!waiting_queue.empty()){
            cpu::self()->ready_queue.push_back(waiting_queue.front());
            owner_id = waiting_queue.front()->thread_id;
            waiting_queue.pop_front();
            // std::cout << "thread " << owner_id << "LOCKED" << std::endl;

            busy = true;
        }

        cpu::guard.exchange(false);
    }

}

void mutex::lock(){
    RAII raii;
    // cpu::interrupt_disable();
    internal_lock();
    // cpu::interrupt_enable();
}

void mutex::unlock(){
    RAII raii;
    // cpu::interrupt_disable();
    if (!busy || owner_id != cpu::self()->current_context_tcb->thread_id) {
        throw std::runtime_error("Attempting to unlock mutex not owned by current thread");
    }
    internal_unlock();
    cpu::send_ipi();
    // cpu::interrupt_enable();
}

