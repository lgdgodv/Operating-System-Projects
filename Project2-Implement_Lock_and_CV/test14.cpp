#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <stdexcept>
using std::cout;
using std::endl;
using std::runtime_error;
int g = 0;

mutex mutex1;
mutex mutex2;


void sb(void *a)
{
    char *id = (char *) a;
    //int i;

    mutex2.lock();

    cout << "bbalbalabala " << id << "///////////////////" << endl;

    mutex2.unlock();
    try
    {
    	mutex1.unlock();
    	mutex2.unlock();
    }
    catch(const runtime_error& error)
    {
    	cout << "error occurs" << endl;
    }

}

void parent(void *a)
{

    thread t1 ( (thread_startfunc_t) sb, (void *) "children thread");

}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
