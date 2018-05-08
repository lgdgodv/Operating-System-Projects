#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <vector>

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;

void wow(void *a)
{
    char *message = (char *) a;
    mutex1.lock();
    cout << "child called with message " << message << ", setting child_done = 1\n";
    mutex1.unlock();
}

void chi(void *a)
{
    char * arg = (char*) a;
    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();
}

void hey(void *a)
{
    char *message = (char *) a;
    cout << "coming\n";
    mutex1.lock();
    cout << "child called with message " << message << ", setting child_done = 1\n";
    mutex1.unlock();
}

void jane(void *a)
{
    char *message = (char *) a;
    for(int i = 0; i < 10; ++i)
    {
        mutex1.lock();
        cout << message << "\n";
        mutex1.unlock();
    }
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;
    mutex1.lock();
    cout << "parent main thread: " << arg << endl;
    mutex1.unlock();

    thread t2 ((thread_startfunc_t) hey, (void *) "hey");
    thread t3 ((thread_startfunc_t) wow, (void *) "wow");
    thread t4 ((thread_startfunc_t) chi, (void *) "chi");
    thread t1 ((thread_startfunc_t) jane, (void *) "jane");
    t1.join();
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
