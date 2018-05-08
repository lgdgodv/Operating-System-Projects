#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <vector>

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
mutex mutex2;
mutex mutex3;
mutex mutex4;
mutex mutex5;
mutex mutex6;
mutex mutex7;
cv cv1;
cv cv2;
int child_done = 0;
int final_done = 0;

void finalOne(void * a)
{
    char *message = (char *) a;
    mutex1.lock();
    //mutex2.lock();
    cout << "final one message: " << message << "\n";
    final_done = 1;
    //mutex2.unlock();
    mutex1.unlock();
}

void lastOne(void * a)
{
    char *message = (char *) a;
    mutex1.lock();
    mutex2.lock();
    cout << "last one message: " << message << "\n";
    mutex2.unlock();
    mutex1.unlock();
    thread t4((thread_startfunc_t) finalOne, (void *) "C");
    t4.join();
    mutex1.lock();
    cout << "t4 finished\n";
    mutex1.unlock();
}

void newOne(void * a)
{
    char *message = (char *) a;
    mutex1.lock();
    mutex2.lock();
    cout << "new one message: " << message << "\n";
    mutex2.unlock();
    mutex1.unlock();
    cout << "after unlock";
    cv1.signal();
    thread t3((thread_startfunc_t) lastOne, (void *) "B");
    t3.join();
    thread::yield();
    t3.join();
    mutex1.lock();
    cout << "t3 finished\n";
    mutex1.unlock();
}

void child_project1(void *a)
{
    char *message = (char *) a;
    cout << "coming\n";
    mutex1.lock();
    cout << "child called with message " << message << ", setting child_done = 1\n";
    child_done = 1;
    //mutex1.unlock();
    cv1.broadcast();

    cv1.broadcast();
    cv1.broadcast();
    cv2.broadcast();
    cv2.broadcast();
    cv2.broadcast();
    cv1.signal();
    cv1.signal();
    cv2.signal();
    cv2.signal();
    cv2.signal();
    mutex1.unlock();

    cv1.signal();
    cv1.broadcast();
    cv2.signal();
    cv2.broadcast();
    mutex1.lock();
    thread::yield();
    mutex1.unlock();

    cout << "back to child\n";
    thread t2((thread_startfunc_t) newOne, (void *) "A");
    t2.join();
    cv2.signal();
    cv2.broadcast();
    t2.join();
}

void parent_project1(void *a)
{
    intptr_t arg = (intptr_t) a;
    cout << "parent_project1\n";
    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t5 ((thread_startfunc_t) child_project1, (void *) "test message");

    mutex1.lock();
    while (!final_done) {
        cout << "parent waiting for child to run\n";
        cv1.wait(mutex1);
        cv1.broadcast();
        cv1.broadcast();
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
    mutex1.lock();
    cout << "here\n";
    mutex1.unlock();
    cout << "before parent yield\n";
    mutex1.lock();
    mutex2.lock();
    mutex3.lock();
    mutex4.lock();
    mutex5.lock();
    mutex6.lock();
    mutex7.lock();
    mutex1.unlock();
    mutex2.unlock();
    mutex3.unlock();
    mutex4.unlock();
    mutex5.unlock();
    mutex6.unlock();
    mutex7.unlock();
    mutex3.lock();
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
