# include "cpu.h"
# include "thread.h"
// # include "tcb.h"
#include <stdio.h>
#include <memory>
#include <ucontext.h>
#include <cassert>


// Static member initializations
std::deque<std::shared_ptr<tcb>> cpu::ready_queue;
std::unordered_map<int, std::shared_ptr<tcb>> cpu::id_to_tcb;
std::unordered_map<int, std::deque<std::shared_ptr<tcb>>> cpu::id_to_joined;
int cpu::next_thread_id = 0;
std::deque<cpu*> cpu::asleep_cpus;
int cpu::cpus = 0;

cpu::cpu(thread_startfunc_t func, uintptr_t arg){
    interrupt_vector_table[0] = &thread::yield;
    interrupt_vector_table[1] = &cpu::ipi_handler;
    cpu_id = cpus++;
    // std::printf("initialized cpu %d\n", cpu::self()->cpu_id);
    // need a cpu id

    if(func != nullptr){
        // make thread
        int thread_id = make_thread(func, arg);
        // push thread to ready queue
        ready_queue.push_back(id_to_tcb[thread_id]);
        send_ipi();

    }

    while(true){
        while(ready_queue.empty()){
            cpu::guard.exchange(false);
            cpu::asleep_cpus.push_back(cpu::self());
            std::printf("putting cpu %d to sleep, waiting for ipi\n", cpu::self()->cpu_id);
            interrupt_enable_suspend();
        }
        std::printf("awoken cpu %d\n", cpu::self()->cpu_id);
        current_context_tcb = ready_queue.front();
        ready_queue.pop_front();
        // switch to next thread
        std::printf("thread %d running on cpu %d\n", cpu::self()->current_context_tcb->thread_id, cpu::self()->cpu_id);

        swapcontext(os_context_ptr.get(), current_context_tcb->ucontext_ptr.get());

        cpu::guard.exchange(false);
    }
 
};

int cpu::make_thread(thread_startfunc_t func, uintptr_t arg){
    auto curr_tcb = std::make_shared<tcb>(next_thread_id, func, arg);
    id_to_tcb[next_thread_id] = curr_tcb;
    return next_thread_id++;
};


void cpu::context_switcher(thread_startfunc_t func, uintptr_t arg){
    // printf("context switcher called!\n");
    std::printf("thread %d running on cpu %d\n", cpu::self()->current_context_tcb->thread_id, cpu::self()->cpu_id);

    // enable interrupts 
    cpu::interrupt_enable();

    // run the thread that runs func
    func(arg);
    
    cpu::interrupt_disable();

    auto curr_cpu = cpu::self();
    // pop all threads who called .join() on current thread, then erase all id instances from memory.
    int curr_thread_id = curr_cpu->current_context_tcb->thread_id;
    // std::printf("ending execution for thread id %d\n", curr_thread_id);
    while(!cpu::id_to_joined[curr_thread_id].empty()){
        cpu::ready_queue.push_back(cpu::id_to_joined[curr_thread_id].front());
        cpu::id_to_joined[curr_thread_id].pop_front();
        cpu::send_ipi();
    }
    cpu::id_to_joined.erase(curr_thread_id);
    cpu::id_to_tcb.erase(curr_thread_id);
    // return to OS
    // if(!cpu::ready_queue.empty()){
    //     curr_cpu->current_context_tcb = cpu::ready_queue.front();
    //     cpu::ready_queue.pop_front();
    //     setcontext(curr_cpu->current_context_tcb->ucontext_ptr.get());
    // } else{
        std::printf("thread %d running on cpu %d\n", cpu::self()->current_context_tcb->thread_id, cpu::self()->cpu_id);
        setcontext(curr_cpu->os_context_ptr.get());
    // }
}   


bool cpu::test_and_set() {
    return guard.exchange(true);
}

void cpu::ipi_handler(){

    cpu::interrupt_disable();
    

    // this isn't atomic, could cause issues?
    if (!cpu::ready_queue.empty()) {
        cpu::self()->current_context_tcb = cpu::ready_queue.front();
        cpu::ready_queue.pop_front();
        
        // cpu::interrupt_enable();
        // std::printf("after enabling\n");
        swapcontext(cpu::self()->os_context_ptr.get(), cpu::self()->current_context_tcb->ucontext_ptr.get());
        // cpu::interrupt_disable();
    }
    // cpu::interrupt_disable();
    // std::printf("after disabling\n");
}


void cpu::send_ipi(){
    // if (asleep_cpus.empty()){
    //     std::printf("ASLEEP_CPUS IS EMPTY????\n");
    // }
    // if (ready_queue.empty()){
    //     std::printf("READY QUEUE IS EMPTY????\n");
    // }

    if (!asleep_cpus.empty() && !ready_queue.empty()){
        auto next_cpu = asleep_cpus.front();
        asleep_cpus.pop_front();
        std::printf("waking cpu %d\n", next_cpu->cpu_id);
        next_cpu->interrupt_send();
        // interrupt_enable_suspend();
    }
}