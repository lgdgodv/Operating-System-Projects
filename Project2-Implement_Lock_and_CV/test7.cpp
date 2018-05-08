
#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;

void hey(void *a)
{
    mutex1.lock();


}

void chi(void *a)
{
    mutex1.lock();
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    //mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    //mutex1.unlock();

    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");

    mutex1.lock();
    t1.join();
    mutex1.unlock();

}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
