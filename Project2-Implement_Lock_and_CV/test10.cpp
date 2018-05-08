//
//  main.cpp
//  eecs482p2
//
//  Created by Eric Yeh on 2018/1/31.
//  Eric Yeh, Junyue Wu, Muting Wu. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;

void loop(void *a)
{
    char *id = (char *) a;
    int i;

    mutex1.lock();
    cout << "loop called with id " << id << endl;

    for (i=0; i<5; i++, g++) {
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
        thread::yield();
        mutex1.lock();
    }
    cout << "back here" << endl;
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void hey(void *a)
{
    char* id = (char *)a;
    cout << "HEY" << id << endl;
}

void chi(void *a)
{
    char* id = (char *)a;
    cout << "chi" << id << endl;
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t3 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t4 ( (thread_startfunc_t) loop, (void *) "hey thread");
    thread t5 ( (thread_startfunc_t) loop, (void *) "hey thread");
    thread t6 ( (thread_startfunc_t) hey, (void *) "chi thread");
    thread t7 ( (thread_startfunc_t) hey, (void *) "chi thread");
    thread t8 ( (thread_startfunc_t) hey, (void *) "chi thread");
    thread t9 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t10 ( (thread_startfunc_t) hey, (void *) "hey thread");

    t1.join();
    t4.join();
    t9.join();
    	                                                                //test for join
    loop( (void *) "parent thread");
    mutex1.lock();
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
