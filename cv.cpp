#include "cv.h"
#include "cpu.h"
#include "raii.h"
#include <stdexcept>
#include <cassert>

cv::cv(){

}

cv::~cv(){

}

void cv::wait(mutex& mtx){
    RAII raii;
    // cpu::interrupt_disable();
    if (!mtx.busy || mtx.owner_id != cpu::self()->current_context_tcb->thread_id) {
        throw std::runtime_error("Calling wait without holding the mutex");
    }
    mtx.internal_unlock();
    waiting_queue.push_back(cpu::self()->current_context_tcb);
    swapcontext(cpu::self()->current_context_tcb->ucontext_ptr.get(), cpu::self()->os_context_ptr.get());
    mtx.internal_lock();
    // cpu::interrupt_enable();
}


void cv::signal(){
    RAII raii;
    // cpu::interrupt_disable();
    if (!waiting_queue.empty()){
        std::shared_ptr<tcb> tcb = waiting_queue.front();
        waiting_queue.pop_front();
        cpu::ready_queue.push_back(tcb);
        // swapcontext(tcb->ucontext_ptr.get(), cpu::self()->current_context_tcb->ucontext_ptr.get());
    }
    cpu::send_ipi();
    // cpu::interrupt_enable();    
}

void cv::broadcast(){
    RAII raii;
    // cpu::interrupt_disable();
    while (!waiting_queue.empty()){
        std::shared_ptr<tcb> tcb = waiting_queue.front();
        waiting_queue.pop_front();
        cpu::ready_queue.push_back(tcb);
        cpu::send_ipi();
    }
    // cpu::interrupt_enable();
}