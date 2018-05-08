#ifndef _CV_H
#define _CV_H

#include "mutex.h"

class cv {
public:
    cv();
    ~cv();

    void wait(mutex&);                  // wait on this condition variable
    void signal();                      // wake up one thread on this condition
                                        // variable
    void broadcast();                   // wake up all threads on this condition
                                        // variable

    class impl;                         // defined by the thread library
    impl *impl_ptr;                     // used by the thread library

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    cv(const cv&) = delete;
    cv& operator=(const cv&) = delete;

    /*
     * Move constructor and move assignment operator.  Implementing these is
     * optional in Project 2.
     */
    cv(cv&&);
    cv& operator=(cv&&);
};

#endif /* _CV_H */
