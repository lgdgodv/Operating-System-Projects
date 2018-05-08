//
//  thread.cpp
//  eecs482p2
//
//  Created by Eric Yeh on 2018/1/31.
//  Copyright © 2018? Eric Yeh. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <queue>
#include <sys/ucontext.h>
#include "thread.h"
#include "impl.cpp"
using namespace std;

std::queue<threadStruct *> ready_q;
std::queue<cpu*> suspend_q;
std::queue<thread::impl*> finished_impl;
int thread_count = 0;

/*
empty the finish queue, can call it every where, to avoid memory leak
but user should take care of when to puth the things in the finish queue
*/
void clear_finish_queue()
{
    while(!finished_impl.empty())
    {
        delete [] (char*)(finished_impl.front()->ucontext_ptr->thread_current->uc_stack.ss_sp);
        delete finished_impl.front()->ucontext_ptr->thread_current;
        delete finished_impl.front()->ucontext_ptr;

        if(finished_impl.front()->ob_dead)
        {
            delete finished_impl.front();
        }
        else
        {
            finished_impl.front()->need_delete = true;
        }

        finished_impl.pop();
    }
}

void timeHandler()
{
    if(cpu::self()->impl_ptr->os != cpu::self()->impl_ptr->cpu_current->thread_current && cpu::self()->impl_ptr->insideos == false)
    {
  		thread::yield();
  	}
}

void ipiHandler()
{

}

/*
wrapper, wrap the real function that need to be excecute, end of this function
means the end of the thread life time
*/
void functors(thread_startfunc_t func, void * arg, thread::impl* impl_ptr)
{
    guard.store(false);
    cpu::self()->interrupt_enable();
    (*func)(arg);
    cpu::self()->interrupt_disable();

    impl_ptr->finished = true;
    while(guard.exchange(true)){}
    impl_ptr->push_list_back();

    clear_finish_queue();

    finished_impl.push(impl_ptr);
    cpu::self()->impl_ptr->yield_to_os(nullptr);
}

thread::thread(thread_startfunc_t func, void * arg)
{
	cpu::self()->interrupt_disable();
	while(guard.exchange(true)){}

	try{
      	impl_ptr = new impl;
        impl_ptr->ucontext_ptr->thread_current = new ucontext_t;
        impl_ptr->ucontext_ptr->id = thread_count;
        thread_count++;

  	    getcontext(impl_ptr->ucontext_ptr->thread_current);
      	char *stack = new char [STACK_SIZE];
        impl_ptr->ucontext_ptr->thread_current->uc_stack.ss_sp = stack;
      	impl_ptr->ucontext_ptr->thread_current->uc_stack.ss_size = STACK_SIZE;
      	impl_ptr->ucontext_ptr->thread_current->uc_stack.ss_flags = 0;
      	impl_ptr->ucontext_ptr->thread_current->uc_link = nullptr;

      	makecontext(impl_ptr->ucontext_ptr->thread_current, (void(*)()) &functors, 3, func, arg, impl_ptr);
        ready_q.push(impl_ptr->ucontext_ptr);
    }
    catch(bad_alloc& error)
    {
        guard.store(false);
        cpu::self()->interrupt_enable();
      	throw(bad_alloc());
    }

    cpu::self()->impl_ptr->suspend_wake_up_helper();
    guard.store(false);
	  cpu::self()->interrupt_enable();

}

void thread::join()
{
    cpu::self()->interrupt_disable();
    while (guard.exchange(true)){}
    threadStruct* temp = cpu::self()->impl_ptr->cpu_current;
    if(impl_ptr == nullptr || impl_ptr->finished == true){}
    else
    {
        impl_ptr->joinwait.push(temp);
        cpu::self()->impl_ptr->yield_to_os(temp->thread_current);
    }
    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
}

void thread::yield()
{
    cpu::self()->interrupt_disable();
    while (guard.exchange(true)){}
    threadStruct* temp = cpu::self()->impl_ptr->cpu_current;
    if(!ready_q.empty())
    {
        ready_q.push(temp);
        cpu::self()->impl_ptr->yield_to_os(temp->thread_current);
    }
    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
}


thread::~thread()
{
    impl_ptr->ob_dead = true;
    if(impl_ptr->need_delete)
    {
        delete impl_ptr;
    }
}
