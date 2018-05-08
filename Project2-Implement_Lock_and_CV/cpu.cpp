#include "cpu.h"
#include <iostream>
#include <queue>
#include "impl.cpp"

//#define _XOPEN_SOURCE
#include <sys/ucontext.h>

using namespace std;

extern std::queue<threadStruct *> ready_q;
extern std::queue<cpu*> suspend_q;
extern std::queue<thread::impl*> finished_impl;
extern void clear_finish_queue();
extern void ipiHandler();
extern void timeHandler();

void cpu::init(thread_startfunc_t func, void * arg)
{
    cpu::self()->impl_ptr = new impl;
    cpu::self()->impl_ptr->cpu_current->thread_current = new ucontext_t;
    cpu::self()->interrupt_vector_table[cpu::TIMER] = &timeHandler;
    cpu::self()->interrupt_vector_table[cpu::IPI] = &ipiHandler;

    if (func)
    {
    	  interrupt_enable();
        thread((thread_startfunc_t) func, (void *) arg);
        interrupt_disable();
    }

    getcontext(impl_ptr->os);
    cpu::self()->impl_ptr->cpu_current->thread_current = cpu::self()->impl_ptr->os;

    while (true)
    {
        while (guard.exchange(true)){}

        if(ready_q.empty())
        {
            suspend_q.push(self());
            guard.store(false);
            impl_ptr->insideos = true;
            interrupt_enable_suspend();
            interrupt_disable();
            while (guard.exchange(true)){}
        }

        if(!ready_q.empty())
        {
            impl_ptr->insideos = false;
            cpu::self()->impl_ptr->cpu_current = ready_q.front();
            ready_q.pop();
            swapcontext(cpu::self()->impl_ptr->os,cpu::self()->impl_ptr->cpu_current->thread_current);
        }

      	clear_finish_queue();
      	guard.store(false);

     }
}
