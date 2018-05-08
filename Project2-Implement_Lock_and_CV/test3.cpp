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
        //cout << "i will be back\n";

        mutex1.lock();
        //cout << "i will be back\n";
    }
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void noYield(void *a)
{
    char *id = (char *) a;
    int i;

    mutex1.lock();
    cout << "noYield function called with id " << id << endl;

    for (i=0; i<5; i++, g++) {
        cout << id << ":\t" << i << "\t" << g << endl;
    }
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void hey(void *a)
{
    char* id = (char *)a;
    int i = 0;
    while(i < 20000){
        i += 1;
    }
    mutex1.lock();
    cout << id << endl;
    mutex1.unlock();
}

void chi(void *a)
{
    char* id = (char *)a;
    mutex1.lock();
    cout << "chi" << id << endl;
    mutex1.unlock();
    thread::yield();
    thread t6 ( (thread_startfunc_t) hey, (void *) "wow thread");
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();
    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");
    t2.join();
    thread t3 ( (thread_startfunc_t) hey, (void *) "children thread");
    t2.join();
    hey( (void *) "parent thread");
    t2.join();

    thread t4((thread_startfunc_t) noYield, (void *) "new thread");
    cout << "t2\n";
    t2.join();
    t1.join();
    t4.join();
    thread t5((thread_startfunc_t) loop, (void *) "another thread");

}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
