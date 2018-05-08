#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <stdexcept>
#include <vector>
using std::cout;
using std::endl;
using std::vector;
using std::bad_alloc;
int g = 0;

mutex mutex1;
cv cv1;
void parent(void *a);

void hey(void *a)
{
    char* id = (char *)a;
    cout << id << endl;
}

void parent(void *a)
{

    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();
  try
  {
        for(int i = 0; i < 9999999; ++i)
        {

                thread t1( (thread_startfunc_t) hey, (void *) "chi thread");

        }
  }
  catch(std::bad_alloc& error)
  {
      cout << "bad allocate error" << endl;
  }
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
