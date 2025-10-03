#include "raii.h"
#include <cassert>
#include "cpu.h"

RAII::RAII() {
    // assert(cpu::inter)
    assert_interrupts_enabled();
    cpu::interrupt_disable();
}

RAII::~RAII() {
    assert_interrupts_disabled();
    cpu::interrupt_enable();
}