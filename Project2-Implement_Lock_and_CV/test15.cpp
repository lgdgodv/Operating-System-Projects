
#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
mutex mutex4;
cv cv1;
cv cv2;


void ba(void *a)
{
    char *id = (char *) a;
    //int i;

    mutex1.lock();

    cout << "bbalbalabala " << id << "///////////////////" << endl;
	while(g != 1){
		cv1.wait(mutex1);
	}
	mutex1.unlock();
    mutex1.unlock();
}

void ma(void *a)
{
    char *id = (char *) a;
    //int i;

    mutex1.lock();

    cout << "bbalbalabala " << id << "///////////////////" << endl;
	  while(g != 1){
		    cv1.wait(mutex1);
	  }
    mutex1.unlock();
}

void sb2(void *a)
{
    char *id = (char *) a;
    //int i;

    mutex1.lock();
    mutex1.lock();
    mutex1.lock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();

    cout << "bbalbalabala " << id << "///////////////////" << endl;
	if(g != 1){
		cv1.wait(mutex1);
	}

}


void sb(void *a)
{
    char *id = (char *) a;
    //int i;

    mutex1.lock();

    cout << "bbalbalabala " << id << "///////////////////" << endl;
	  cv1.signal();
  	cv1.signal();
    mutex1.unlock();
}

void parent(void *a)
{

    //intptr_t arg = (intptr_t) a;


    thread t1 ( (thread_startfunc_t) ba, (void *) "children thread");
    thread t2 ( (thread_startfunc_t) ma, (void *) "chiken thread");
    thread t3 ( (thread_startfunc_t) sb, (void *) "sb2 thread");
    thread t4 ( (thread_startfunc_t) sb2, (void *) "sb thread");

}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
