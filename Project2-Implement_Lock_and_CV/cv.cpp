//
//  cv.cpp
//  eecs482p2
//
//  Created by Eric Yeh on 2018/1/31.
//  Copyright © 2018年 Eric Yeh. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <queue>
#include <ucontext.h>
#include "thread.h"
#include "cv.h"
#include "impl.cpp"
#include <stdexcept>

extern std::queue<threadStruct *> ready_q;
extern std::queue<thread::impl*> finished_impl;
extern void clear_finish_queue();
using std::cout;
using std::endl;

cv::cv()
{
    impl_ptr = new impl;
}

void cv::wait(mutex& lock)
{
    cpu::self()->interrupt_disable();

    while (guard.exchange(true)){}

    threadStruct* temp = cpu::self()->impl_ptr->cpu_current;
    impl_ptr->cv_waiting.push(temp);
    lock.impl_ptr->unlock_helper();
    cpu::self()->impl_ptr->yield_to_os(temp->thread_current);
    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
    lock.lock();
}



void cv::signal()
{
    cpu::self()->interrupt_disable();

    while (guard.exchange(true)){}


    if(!impl_ptr->cv_waiting.empty())
    {
        impl_ptr->cv_signal_hleper();
    }

    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
}

void cv::broadcast()
{
    cpu::self()->interrupt_disable();
    while (guard.exchange(true)){}
    while(!impl_ptr->cv_waiting.empty())
    {
        impl_ptr->cv_signal_hleper();
    }

    clear_finish_queue();
    guard.store(false);
    cpu::self()->interrupt_enable();
}

cv::~cv()
{
    delete impl_ptr;
}
