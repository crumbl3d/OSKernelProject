/*
 * KThread.h
 * 
 * Created on: May 17, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _KTHREAD_H_
#define _KTHREAD_H_

#include "Thread.h"

typedef void (*pBody)(); // run method type

const StackSize minStackSize = 128; // min = 128B
const StackSize maxStackSize = 65535; // max = 64KB

const unsigned initialObjectCapacity = 100; // initial capacity

struct ThreadState { enum ThreadStateEnum { New, Ready, Running, Blocked, Terminated }; };

class PCB
{
public:
    // for creating initial context
    PCB();

    // body - method to run
    // stackSize - in BYTE (8b), quantum - in x55ms
    PCB(pBody body, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    // thread - user thread this object is created for
    PCB(Thread *thread, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    virtual ~PCB();

    void start();
    void waitToComplete();

    void setTimeSlice(Time timeSlice);

    static void sleep(unsigned timeToSleep);

    static PCB* getAt(unsigned index);
private:
    friend class System;

    // Common initialization
    static void initialize(PCB *kernelThread, Thread *userThread, pBody body,
                           StackSize stackSize, Time timeSlice);

    // Thread context
    unsigned *mStack;
    unsigned mSS;
    unsigned mSP;
    unsigned mBP;
    unsigned mTimeSlice;
    ThreadState::ThreadStateEnum mState;

    // System data: mThread (pointer to the user thread),
    //              mNext (pointer to the next kernel thread)
    //              mID (unique ID of this kernel thread)
    Thread *mThread;
    PCB *mNext;
    ID mID;
    
    // Class data: capacity (current maximum capacity of the object array)
    //             count (number of created objects)
    //             objects (array of all the created objects of this class)
    static unsigned capacity, count;
    static PCB **objects;
};

#endif /* _KTHREAD_H_ */