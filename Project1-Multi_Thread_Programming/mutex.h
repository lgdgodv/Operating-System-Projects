#ifndef _MUTEX_H
#define _MUTEX_H

class mutex {
public:
    mutex();
    ~mutex();

    void lock();
    void unlock();

    class impl;                                 // defined by the thread library
    impl *impl_ptr;                             // used by the thread library

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    /*
     * Move constructor and move assignment operator.
     */
    mutex(mutex&&);
    mutex& operator=(mutex&&);
};

#endif /* _MUTEX_H */
