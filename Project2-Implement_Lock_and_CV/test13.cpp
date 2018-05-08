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
cv cv1;
cv cv2;
bool signalVal = false;

void jane(void * a)
{
    char *id = (char *) a;
    mutex2.lock();
    cout << "bbalbalabala " << id << "///////////////////" << endl;
    mutex2.unlock();
    mutex1.lock();
    while(signalVal != true){
        cv2.wait(mutex1);
    }
    cv1.signal();
}

void eric(void *a)
{
    char *id = (char *) a;
    mutex2.lock();
    cout << "bbalbalabala " << id << "///////////////////" << endl;
    mutex2.unlock();
    try
    {
        mutex1.lock();
        signalVal = true;
        cv1.wait(mutex2);
        cv2.signal();
        mutex1.unlock();
    }
    catch(const runtime_error& error){
        cout << "error occurs" << endl;
    }

}

void parent(void *a)
{
    thread t1 ( (thread_startfunc_t) eric, (void *) "children thread");
    thread t2 ( (thread_startfunc_t) jane, (void *) "sb thread");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
