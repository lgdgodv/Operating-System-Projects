#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <vector>

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;
int child_done = 0;		// global variable; shared between the two threads.

void loop(void *a)
{

    //cout << "loop insider\n" << endl;
    char *id = (char *) a;
    int i;

    mutex1.lock();

    cout << "loop called with id " << id << "///////////////////" << endl;

    for (i=0; i<5; i++, g++) {
      //cout << "for loop: " <<  i << endl;
        cout << id << ":\t" << i << "\t" << g << endl;
        mutex1.unlock();
        //thread::yield();
        mutex1.lock();
    }
    cout << id << ":\t" << i << "\t" << g << endl;
    mutex1.unlock();
}

void balabala(void *a)
{
    char *id = (char *) a;
    int i;

    mutex1.lock();

    cout << "loop called with id " << id << "///////////////////" << endl;

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
    mutex1.lock();
    cout << id << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    while(i < 10000)
    {
        i++;
    }
    mutex1.unlock();
    thread::yield();
    mutex1.lock();
    cout << "back after" << endl;
    mutex1.unlock();
}

void chi(void *a)
{
    char* id = (char *)a;
    mutex1.lock();
    cout << "chi" << id << "------------------------" << endl;
    thread t4 ( (thread_startfunc_t) balabala, (void *) "new");
    mutex1.unlock();
    t4.join();
    cout << "chi finish waiting for t4\n";

}


void child_project1(void *a)
{
    char *message = (char *) a;
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
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t1 ( (thread_startfunc_t) chi, (void *) "chi thread");
    thread t2 ( (thread_startfunc_t) hey, (void *) "hey thread");
    thread t3 ( (thread_startfunc_t) loop, (void *) "wow thread");

    t1.join();
    t2.join();


    //test for join
    loop( (void *) "parent thread");

    thread t6 ((thread_startfunc_t) parent_project1, (void *) 100);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
