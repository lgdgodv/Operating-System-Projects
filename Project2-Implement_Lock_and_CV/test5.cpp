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
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void looper(void *a)
{
    char *id = (char *) a;
    int i;

    mutex1.lock();
    cout << "loop called with id " << id << endl;

    for (i=0; i<5; i++, g++) {
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
        //thread::yield();
        mutex1.lock();
    }
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void hey(void *a)
{
    char* id = (char *)a;
    mutex1.lock();
    cout << "HEY" << id << endl;
    mutex1.unlock();
}

void chi(void *a)
{
    char* id = (char *)a;
        mutex1.lock();
    cout << "chi" << id << endl;
        mutex1.unlock();
    thread t2 ( (thread_startfunc_t) hey, (void *) "jane");
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t3 ( (thread_startfunc_t) looper, (void *) "children thread");

    t1.join();
    looper((void *) "parent thread");
    thread t4 ( (thread_startfunc_t) looper, (void *) "grandson thread");
    thread t5 ( (thread_startfunc_t) looper, (void *) "grandma thread");
    t5.join();

    thread t6 ( (thread_startfunc_t) loop, (void *) "grandpa thread");
    t2.join();

    thread t7 ( (thread_startfunc_t) looper, (void *) "grandson2 thread");
    looper((void *) "parent threads");
    thread t8 ( (thread_startfunc_t) looper, (void *) "grandma2 thread");
    t8.join();
    cout << "hi";
    thread t9 ( (thread_startfunc_t) looper, (void *) "grandpa2 thread");
    cout << "heyhoho";
    t7.join();
    //cout << "hey";

}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 100);
}
