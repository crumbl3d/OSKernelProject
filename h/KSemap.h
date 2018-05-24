/*
 * KSemap.h
 * 
 * Created on: May 24, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _KSEMAP_H_
#define _KSEMAP_H_

#include "Semaphor.h"

class PCB;
class PCBQueue;

class KernelSem
{
public:
    // Used for creating kernel semaphores. Parameters:
    //      init - initial semaphore value
    KernelSem (int init = 1);
    
    // Used for creating user semaphores. Parameters:
    //      userSem - user semaphore this object is created for
    //      init - initial semaphore value
    KernelSem (Semaphore *userSem, int init = 1);

    virtual ~KernelSem ();

    virtual int wait (int toBlock);
    virtual void signal ();

    int val () const;
protected:
    void block ();
    void deblock ();
private:
    friend class System;

    // Common initialization
    void initialize (Semaphore *userSem, int init);

    // Semaphore value
    int mValue;

    // System data: mSemaphore (pointer to the user semaphore),
    //              mBlocked (list of threads blocked on this one)
    //              mID (unique ID of this kernel semaphore)
    Semaphore *mSemaphore;
    PCBQueue *mBlocked;
    ID mID;

    // Class data: capacity (current maximum capacity of the object array)
    //             count (number of created objects)
    //             objects (array of all the created objects of this class)
    static unsigned capacity, count;
    static KernelSem **objects;
};

#endif /* _KSEMAP_H_ */