//
//  impl.cpp
//  eecs482p2
//
//  Created by Eric Yeh on 2018/2/1.
//  Copyright © 2018年. All rights reserved.
//

#include <stdio.h>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include <ucontext.h>
#include <iostream>
#include <queue>
#include <atomic>
#include <stdexcept>

class threadStruct
{
public:
    ucontext_t* thread_current;
    int id;
};

extern std::queue<threadStruct *> ready_q;
extern std::queue<cpu*> suspend_q;
extern std::queue<thread::impl*> finished_impl;
using std::cout;


class cpu::impl
{

public:
    ucontext_t * os = new ucontext_t;
    threadStruct * cpu_current = new threadStruct;
	  bool insideos = true;
    void suspend_wake_up_helper()
    {
        if(!suspend_q.empty())
        {
            cpu* temp = suspend_q.front();
            suspend_q.pop();
            temp->interrupt_send();
        }
    }


    /*
    this hleper take a ucontext pointer and go to the next ready thread
    can pass in a nullptr to this helper, which means the thread is done and
    dont need to swap
    */
    void yield_to_os(ucontext_t* temp)
    {
        if(ready_q.empty())
        {
            if(temp)
            {
            	insideos = true;
                swapcontext(temp,os);
            }
            else
            {
            	insideos = true;
                setcontext(os);
            }
        }
        else
        {
            cpu_current = ready_q.front();
            ready_q.pop();
            if(temp)
            {
                swapcontext(temp,cpu_current->thread_current);

            }
            else
            {
                setcontext(cpu_current->thread_current);
            }
        }
    }
};


class thread::impl
{
public:
	std::queue<threadStruct*> joinwait;
    threadStruct * ucontext_ptr = new threadStruct;
    bool finished = false;
    bool ob_dead = false;
    bool need_delete = false;
    // push the joinwait back
    void push_list_back()
    {
    	  while(!joinwait.empty())
        {
      	    ready_q.push(joinwait.front());
      	    joinwait.pop();
      	    cpu::self()->impl_ptr->suspend_wake_up_helper();
    	  }
    }
};




class mutex::impl
{
public:
    int thread_id;
    bool free = true;
    std::queue<threadStruct*> lock_waiting;

    // thies function can be call in mutext and cv
    void unlock_helper()
    {
        if(thread_id != cpu::self()->impl_ptr->cpu_current->id)
        {
            guard.store(false);
            cpu::self()->interrupt_enable();
            throw std::runtime_error("error");
        }
        free = true;
        if (!lock_waiting.empty())
        {
            thread_id = lock_waiting.front() -> id;
            ready_q.push(lock_waiting.front());
            lock_waiting.pop();
            free = false;
            cpu::self()->impl_ptr->suspend_wake_up_helper();
        }
        else
        {
            thread_id = -1;
        }
    }
};






class cv::impl
{
public:
    bool free = true;
    std::queue<threadStruct*> cv_waiting;

    // in the broadcast and signal, will cahll this functi0n
    void cv_signal_hleper()
    {
        threadStruct* temp = cpu::self()->impl_ptr->cpu_current;
        temp = cv_waiting.front();
        cv_waiting.pop();
        ready_q.push(temp);
        cpu::self()->impl_ptr->suspend_wake_up_helper();
    }
};
