//
//  mutex.cpp
//  eecs482p2
//
//  Created by Eric Yeh on 2018/1/31.
//  Copyright © 2018年 Eric Yeh. All rights reserved.
//

#include <stdio.h>
#include "mutex.h"
#include "impl.cpp"
#include "cpu.h"
#include <atomic>
#include <queue>
#include <stdexcept>
using std::cout;
using std::endl;
extern std::queue<threadStruct *> ready_q;
extern std::queue<thread::impl*> finished_impl;
extern void clear_finish_queue();
mutex::mutex()
{
    impl_ptr = new impl;
    impl_ptr->thread_id = -1;
}

mutex::~mutex()
{
    delete impl_ptr;
}

void mutex::lock()
{

    cpu::self()->interrupt_disable();
    while (guard.exchange(true)){}
    if (impl_ptr->free)
    {
        impl_ptr->free = false;
        impl_ptr->thread_id = cpu::self()->impl_ptr->cpu_current->id;
    }
    else
    {
        threadStruct* temp = cpu::self()->impl_ptr->cpu_current;
        impl_ptr->lock_waiting.push(temp);
        cpu::self()->impl_ptr->yield_to_os(temp->thread_current);
    }
    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
}

void mutex::unlock()
{
    cpu::self()->interrupt_disable();
    while (guard.exchange(true)){}

    impl_ptr->unlock_helper();

  	 clear_finish_queue();
     guard.store(false);
     cpu::self()->interrupt_enable();

}
