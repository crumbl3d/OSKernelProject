/*
 * KThread.h
 * 
 * Created on: May 17, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _KTHREAD_H_
#define _KTHREAD_H_

#include "Thread.h"

typedef void (*ThreadBody)(); // run method type

const StackSize minStackSize = 128; // min = 128B
const StackSize maxStackSize = 65535; // max = 64KB

const unsigned initialObjectCapacity = 100; // initial capacity

struct ThreadState { enum ThreadStateEnum { New, Ready, Running, Blocked, Terminated }; };

class PCB
{
public:
    // Used for creating the initial context.
    PCB();

    // Used for creating kernel threads. Parameters:
    // body - method to run
    // stackSize - size of the stack in BYTE (8b)
    // timeSlice - time to run in x55ms
    PCB(ThreadBody body, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    // Used for creating user threads. Parameters:
    // thread - user thread this object is created for
    // stackSize - size of the stack in BYTE (8b)
    // timeSlice - time to run in x55ms
    PCB(Thread *userThread, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    virtual ~PCB();

    void start();
    void waitToComplete();

    static void stop();
    static void sleep(unsigned timeToSleep);

    static PCB* getAt(unsigned index);
private:
    friend class System;

    // Common initialization
    void initialize(Thread *userThread, ThreadBody body,
                    StackSize stackSize, Time timeSlice);

    // Thread context
    unsigned *mStack;
    unsigned mSS;
    unsigned mSP;
    unsigned mBP;

    // System data: mTimeSlice (number of timer ticks this thread will run)
    //              mTimeLeft (number of timer ticks left to sleep)
    //              mThread (pointer to the user thread),
    //              mNext (pointer to the next kernel thread)
    //              mBlocked (list of threads blocked on this one)
    //              mID (unique ID of this kernel thread)
    Time mTimeSlice, mTimeLeft;
    ThreadState::ThreadStateEnum mState;
    Thread *mThread;
    PCB *mNext, *mBlocked;
    ID mID;
    
    // Class data: capacity (current maximum capacity of the object array)
    //             count (number of created objects)
    //             objects (array of all the created objects of this class)
    static unsigned capacity, count;
    static PCB **objects;
};

#endif /* _KTHREAD_H_ */