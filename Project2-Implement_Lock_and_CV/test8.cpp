#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

void loop(void *a);
//thread t10 ( (thread_startfunc_t) loop, (void *) "grandpapapapapa thread");

int g = 0;
mutex mutex2;
mutex mutex1;
mutex mutex3;
cv cv1;

void loop(void *a)
{
    char *id = (char *) a;
    int i;

    mutex1.lock();
    cout << "loop called with id " << id << endl;
    mutex1.unlock();
    for (i=0; i<5; i++, g++) {
    	mutex1.lock();
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
        thread::yield();
    }
    mutex1.lock();
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void looper(void *a)
{
    char *id = (char *) a;
    int i;
    mutex1.lock();
    cout << "loop called with id " << id << endl;
    mutex1.unlock();
    for (i=0; i<5; i++, g++) {
    	mutex1.lock();
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
    }
    mutex1.lock();
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}


void loopers(void *a)
{
    char *id = (char *) a;
    int i;
    mutex1.lock();
    cout << "loop called with id " << id << endl;
    mutex1.unlock();
    for (i=0; i<5; i++, g++) {
    	mutex1.lock();
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
    }
    mutex1.lock();
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void hey(void *a)
{
    char* id = (char *)a;
    thread::yield();
    mutex2.lock();
    cout << "HEY" << id << endl;
    mutex2.unlock();
}

void chi(void *a)
{
    char* id = (char *)a;
    mutex3.lock();
    cout << "chi" << id << endl;
    mutex3.unlock();
    thread t11 ( (thread_startfunc_t) looper, (void *) "grandpapapapapa thread");
    t11.join();
    thread::yield();
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey second time thread");
    t2.join();
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;
    mutex2.lock();
    cout << "parent called with arg " << arg << endl;
    mutex2.unlock();
    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t3 ( (thread_startfunc_t) loop, (void *) "children thread");

    t1.join();
    loop((void *) "parent thread");
    thread t4 ( (thread_startfunc_t) loop, (void *) "grandson thread");
    thread t5 ( (thread_startfunc_t) loop, (void *) "grandma thread");
    t5.join();
    thread t6 ( (thread_startfunc_t) loop, (void *) "grandpa thread");
    t2.join();

    thread t7 ( (thread_startfunc_t) looper, (void *) "grandson2 thread");
    looper((void *) "parent threads");
    thread t8 ( (thread_startfunc_t) looper, (void *) "grandma2 thread");
    t8.join();
    thread t9 ( (thread_startfunc_t) looper, (void *) "grandpa2 thread");
    t7.join();

    thread t16 ( (thread_startfunc_t) loopers, (void *) "grandpa2 thread");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
