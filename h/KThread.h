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

class PCB
{
public:
    enum State { New, Ready, Running, Blocked, Terminated };

    // for creating initial context
    PCB();

    // thread - if created by a Thread object,
    // stackSize - in BYTE (8b), quantum - in x55ms
    PCB(Thread *thread, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    // body - method to run
    PCB(pBody body, StackSize stackSize = defaultStackSize,
        Time timeSlice = defaultTimeSlice);

    virtual ~PCB();

    void start();
    void waitToComplete();

    void setTimeSlice(Time timeSlice);

    static void sleep(unsigned timeToSleep);
    static void dispatch();
private:
    friend class System;

    // Thread Control Block - Thread context
    unsigned *mStack;
    unsigned mSS;
    unsigned mSP;
    unsigned mBP;
    unsigned mTimeSlice;
    State mState;

    // Link to the user thread and next kernel thread
    Thread *mThread;
    PCB *mNext;
};

#endif /* _KTHREAD_H_ */