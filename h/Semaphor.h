/*
 * Semaphor.h
 * 
 * Created on: May 24, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _SEMAPHOR_H_
#define _SEMAPHOR_H_

typedef int ID;

class KernelSem; // Kernel's implementation of a user's semaphore

class Semaphore
{
public:
    Semaphore (int init = 1);
    virtual ~Semaphore ();

    virtual int wait (int toBlock);
    virtual void signal ();

    int val () const; // Returns the current value of the semaphore
private:
    ID mID;
};

#endif /* _SEMAPHOR_H_ */