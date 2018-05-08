#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <vector>

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;
int child_done = 0;

void child_project1(void *a)
{
    char *message = (char *) a;
    cout << "coming\n";
    mutex1.lock();
    cout << "child called with message " << message << ", setting child_done = 1\n";
    child_done = 1;
    cv1.signal();
    mutex1.unlock();
}

void parent_project1(void *a)
{
    intptr_t arg = (intptr_t) a;
    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t5 ((thread_startfunc_t) child_project1, (void *) "test message");

    mutex1.lock();
    while (!child_done) {
        cout << "parent waiting for child to run\n";
        cv1.wait(mutex1);
    }
    cout << "parent finishing" << endl;
    mutex1.unlock();
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent main thread: " << arg << endl;
    mutex1.unlock();

    thread t6 ((thread_startfunc_t) parent_project1, (void *) 100);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
